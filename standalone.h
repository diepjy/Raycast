#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector_types.h>
#include <cutil_math.h>


void init(){};
void clean(){};


#define INVALID -2
// DATA TYPE


template<typename T>
void read_input(std::string inputfile, T * in) {
	size_t isize;
	std::ifstream file(inputfile.c_str(),
			std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		isize = file.tellg();
		file.seekg(0, std::ios::beg);
		file.read((char*) in, isize);
		file.close();
	} else {
		std::cout << "File opening failed : " << inputfile << std::endl;
		exit(1);
	}
}


struct Volume {
	uint3 size;
	float3 dim;
	short2 * data;

	Volume() {
		size = make_uint3(0);
		dim = make_float3(1);
		data = NULL;
	}

	float2 operator[](const uint3 & pos) const {
		const short2 d = data[pos.x + pos.y * size.x + pos.z * size.x * size.y];
		return make_float2(d.x * 0.00003051944088f, d.y); //  / 32766.0f
	}

	float v(const uint3 & pos) const {
		return operator[](pos).x;
	}

	float vs(const uint3 & pos) const {
		return data[pos.x + pos.y * size.x + pos.z * size.x * size.y].x;
	}
	inline float vs2(const uint x, const uint y, const uint z) const {
		return data[x + y * size.x + z * size.x * size.y].x;
	}

	void setints(const unsigned x, const unsigned y, const unsigned z,
			const float2 &d) {
		data[x + y * size.x + z * size.x * size.y] = make_short2(d.x * 32766.0f,
				d.y);
	}

	void set(const uint3 & pos, const float2 & d) {
		data[pos.x + pos.y * size.x + pos.z * size.x * size.y] = make_short2(
				d.x * 32766.0f, d.y);
	}
	float3 pos(const uint3 & p) const {
		return make_float3((p.x + 0.5f) * dim.x / size.x,
				(p.y + 0.5f) * dim.y / size.y, (p.z + 0.5f) * dim.z / size.z);
	}

	float interp(const float3 & pos) const {

		const float3 scaled_pos = make_float3((pos.x * size.x / dim.x) - 0.5f,
				(pos.y * size.y / dim.y) - 0.5f,
				(pos.z * size.z / dim.z) - 0.5f);
		const int3 base = make_int3(floorf(scaled_pos));
		const float3 factor = fracf(scaled_pos);
		const int3 lower = max(base, make_int3(0));
		const int3 upper = min(base + make_int3(1),
				make_int3(size) - make_int3(1));
		return (((vs2(lower.x, lower.y, lower.z) * (1 - factor.x)
				+ vs2(upper.x, lower.y, lower.z) * factor.x) * (1 - factor.y)
				+ (vs2(lower.x, upper.y, lower.z) * (1 - factor.x)
						+ vs2(upper.x, upper.y, lower.z) * factor.x) * factor.y)
				* (1 - factor.z)
				+ ((vs2(lower.x, lower.y, upper.z) * (1 - factor.x)
						+ vs2(upper.x, lower.y, upper.z) * factor.x)
						* (1 - factor.y)
						+ (vs2(lower.x, upper.y, upper.z) * (1 - factor.x)
								+ vs2(upper.x, upper.y, upper.z) * factor.x)
								* factor.y) * factor.z) * 0.00003051944088f;

	}

	float3 grad(const float3 & pos) const {
		const float3 scaled_pos = make_float3((pos.x * size.x / dim.x) - 0.5f,
				(pos.y * size.y / dim.y) - 0.5f,
				(pos.z * size.z / dim.z) - 0.5f);
		const int3 base = make_int3(floorf(scaled_pos));
		const float3 factor = fracf(scaled_pos);
		const int3 lower_lower = max(base - make_int3(1), make_int3(0));
		const int3 lower_upper = max(base, make_int3(0));
		const int3 upper_lower = min(base + make_int3(1),
				make_int3(size) - make_int3(1));
		const int3 upper_upper = min(base + make_int3(2),
				make_int3(size) - make_int3(1));
		const int3 & lower = lower_upper;
		const int3 & upper = upper_lower;

		float3 gradient;

		//#pragma omp parallel sections
		{
		//#pragma omp section
		{
		gradient.x = (((vs2(upper_lower.x, lower.y, lower.z)
				- vs2(lower_lower.x, lower.y, lower.z)) * (1 - factor.x)
				+ (vs2(upper_upper.x, lower.y, lower.z)
						- vs2(lower_upper.x, lower.y, lower.z)) * factor.x)
				* (1 - factor.y)
				+ ((vs2(upper_lower.x, upper.y, lower.z)
						- vs2(lower_lower.x, upper.y, lower.z)) * (1 - factor.x)
						+ (vs2(upper_upper.x, upper.y, lower.z)
								- vs2(lower_upper.x, upper.y, lower.z))
								* factor.x) * factor.y) * (1 - factor.z)
				+ (((vs2(upper_lower.x, lower.y, upper.z)
						- vs2(lower_lower.x, lower.y, upper.z)) * (1 - factor.x)
						+ (vs2(upper_upper.x, lower.y, upper.z)
								- vs2(lower_upper.x, lower.y, upper.z))
								* factor.x) * (1 - factor.y)
						+ ((vs2(upper_lower.x, upper.y, upper.z)
								- vs2(lower_lower.x, upper.y, upper.z))
								* (1 - factor.x)
								+ (vs2(upper_upper.x, upper.y, upper.z)
										- vs2(lower_upper.x, upper.y, upper.z))
										* factor.x) * factor.y) * factor.z;
		}
		//#pragma omp section 
		{

		gradient.y = (((vs2(lower.x, upper_lower.y, lower.z)
				- vs2(lower.x, lower_lower.y, lower.z)) * (1 - factor.x)
				+ (vs2(upper.x, upper_lower.y, lower.z)
						- vs2(upper.x, lower_lower.y, lower.z)) * factor.x)
				* (1 - factor.y)
				+ ((vs2(lower.x, upper_upper.y, lower.z)
						- vs2(lower.x, lower_upper.y, lower.z)) * (1 - factor.x)
						+ (vs2(upper.x, upper_upper.y, lower.z)
								- vs2(upper.x, lower_upper.y, lower.z))
								* factor.x) * factor.y) * (1 - factor.z)
				+ (((vs2(lower.x, upper_lower.y, upper.z)
						- vs2(lower.x, lower_lower.y, upper.z)) * (1 - factor.x)
						+ (vs2(upper.x, upper_lower.y, upper.z)
								- vs2(upper.x, lower_lower.y, upper.z))
								* factor.x) * (1 - factor.y)
						+ ((vs2(lower.x, upper_upper.y, upper.z)
								- vs2(lower.x, lower_upper.y, upper.z))
								* (1 - factor.x)
								+ (vs2(upper.x, upper_upper.y, upper.z)
										- vs2(upper.x, lower_upper.y, upper.z))
										* factor.x) * factor.y) * factor.z;

		}

		//#pragma omp section 
		{
		gradient.z = (((vs2(lower.x, lower.y, upper_lower.z)
				- vs2(lower.x, lower.y, lower_lower.z)) * (1 - factor.x)
				+ (vs2(upper.x, lower.y, upper_lower.z)
						- vs2(upper.x, lower.y, lower_lower.z)) * factor.x)
				* (1 - factor.y)
				+ ((vs2(lower.x, upper.y, upper_lower.z)
						- vs2(lower.x, upper.y, lower_lower.z)) * (1 - factor.x)
						+ (vs2(upper.x, upper.y, upper_lower.z)
								- vs2(upper.x, upper.y, lower_lower.z))
								* factor.x) * factor.y) * (1 - factor.z)
				+ (((vs2(lower.x, lower.y, upper_upper.z)
						- vs2(lower.x, lower.y, lower_upper.z)) * (1 - factor.x)
						+ (vs2(upper.x, lower.y, upper_upper.z)
								- vs2(upper.x, lower.y, lower_upper.z))
								* factor.x) * (1 - factor.y)
						+ ((vs2(lower.x, upper.y, upper_upper.z)
								- vs2(lower.x, upper.y, lower_upper.z))
								* (1 - factor.x)
								+ (vs2(upper.x, upper.y, upper_upper.z)
										- vs2(upper.x, upper.y, lower_upper.z))
										* factor.x) * factor.y) * factor.z;
		}
		}
		return gradient
				* make_float3(dim.x / size.x, dim.y / size.y, dim.z / size.z)
				* (0.5f * 0.00003051944088f);
	}

	void init(uint3 s, float3 d) {
		size = s;
		dim = d;
		data = (short2 *) malloc(size.x * size.y * size.z * sizeof(short2));
		assert(data != NULL);

	}

	void release() {
		free(data);
		data = NULL;
	}
};

typedef struct sMatrix4 {
	float4 data[4];
} Matrix4;

inline float3 get_translation(const Matrix4 view) {
	return make_float3(view.data[0].w, view.data[1].w, view.data[2].w);
}

inline float3 rotate(const Matrix4 & M, const float3 & v) {
	return make_float3(dot(make_float3(M.data[0]), v),
			dot(make_float3(M.data[1]), v), dot(make_float3(M.data[2]), v));
}
