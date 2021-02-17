#version 460

 

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
layout(rg16, binding = 0) uniform image3D volume;


  layout(std430, binding = 2) buffer configLUT{ int cLUT[]; };	
 layout(std430, binding = 3) buffer triangleVertices{ float tVs[]; };	

 uniform float IsoLevel;
 uniform int McFactor;
 uniform ivec3 VolumeDims;
 
uint getCornerA(int edge){
    return uint(edge % 8);
}

uint getCornerB(int edge){
	if(edge==0)
		return 1u;
	if(edge==1)
		return 2u;
	if(edge==2)
		return 3u;
	if(edge==3)
		return 0u;
	if(edge==4)
		return 5u;
	if(edge==5)
		return 6u;
	if(edge==6)
		return 7u;
	if(edge==7)
		return 4u;
	if(edge==8)
		return 4u;
	if(edge==9)
		return 5u;
	if(edge==10)
		return 6u;
	if(edge==11)
		return 7u;
}
vec3 interpolateVerts(vec4 v1, vec4 v2) {
//	float t = (v1.w - IsoLevel) / (v2.w - v1.w);
//    return v1.xyz + t * (v2.xyz-v1.xyz);
//

	return (v1.xyz+v2.xyz)*0.5f;
	
}

uint[9] getTriangleVerticeIndeces(uint volIndex1D, int edgeIndex){
	uint indecesToReturn[9];
	for(int i = 0; i < 9; i++){
		//indecesToReturn[i] = (uint(volIndex1D)/McFactor)+uint(edgeIndex*3)+uint(i);
		indecesToReturn[i] = uint(volIndex1D*45)+uint(edgeIndex*3)+uint(i);
	}
	return indecesToReturn;
}

void getIDs(ivec3 idx){

	//uint arraySize =  uint((VolumeDims.x / McFactor) * (VolumeDims.y / McFactor) * (VolumeDims.z / McFactor));
	uint volIndex1D = uint(idx.z/McFactor * VolumeDims.x/McFactor * VolumeDims.y/McFactor + idx.y/McFactor * VolumeDims.x/McFactor + idx.x/McFactor);
	//uint volIndex1D = uint(idx.z * VolumeDims.x * VolumeDims.y + idx.y * VolumeDims.x + idx.x);

	//Skip values between cubes
	if (idx.x % McFactor != 0 && idx.y % McFactor != 0 && idx.z % McFactor == -1)
		return;

	//Organ Selection
	if(uint(imageLoad(volume, idx).g * 65535.0) == -1)
		return;

	//Get corner coordinates
	vec4 cubeCornersIdx[8];
	cubeCornersIdx[0].xyz = vec3(idx.x, idx.y, idx.z+McFactor);
	cubeCornersIdx[1].xyz = vec3(idx.x+McFactor, idx.y, idx.z+McFactor);
	cubeCornersIdx[2].xyz = vec3(idx.x+McFactor, idx.y, idx.z);
	cubeCornersIdx[3].xyz = vec3(idx.x, idx.y, idx.z);
	cubeCornersIdx[4].xyz = vec3(idx.x, idx.y+McFactor, idx.z+McFactor);
	cubeCornersIdx[5].xyz = vec3(idx.x+McFactor, idx.y+McFactor, idx.z+McFactor);
	cubeCornersIdx[6].xyz = vec3(idx.x+McFactor, idx.y+McFactor, idx.z);
	cubeCornersIdx[7].xyz = vec3(idx.x, idx.y+McFactor, idx.z);

	//Get cube configuration
	uint cubeIndex = 0u;
	for(int i = 0; i < 8; i++){
		cubeCornersIdx[i].w = imageLoad(volume, ivec3(cubeCornersIdx[i].xyz)).g * 65535.0;
		//cubeCornersIdx[i].w = imageLoad(volume, ivec3(cubeCornersIdx[i].xyz)).r;
		if (cubeCornersIdx[i].w == 4){
			uint value =  uint(pow(2u,uint(i))); 
			cubeIndex = cubeIndex | value;
		}
	}
	if(cubeIndex == 0u || cubeIndex == 255u)
		return;

	//Get edges
	uint cLUTIndex = cubeIndex*16u;
	int edges[16];

	for(int i = 0; i < 16; i++){
		edges[i] = cLUT[cLUTIndex+i];
	}
	
	
	for(int i = 0; edges[i] != -1; i+=3){
		
		
		uint corner1 = getCornerA(edges[i]);
		uint corner2 = getCornerB(edges[i]);

		uint corner3 = getCornerA(edges[i+1]);
		uint corner4 = getCornerB(edges[i+1]);
//
		uint corner5 = getCornerA(edges[i+2]);
		uint corner6 = getCornerB(edges[i+2]);


		vec3 vertA = interpolateVerts(cubeCornersIdx[corner1], cubeCornersIdx[corner2]);
		vec3 vertB = interpolateVerts(cubeCornersIdx[corner3], cubeCornersIdx[corner4]);
		vec3 vertC = interpolateVerts(cubeCornersIdx[corner5], cubeCornersIdx[corner6]);




		uint triangleVerticleIndeces[9] = getTriangleVerticeIndeces(volIndex1D, i);



		tVs[triangleVerticleIndeces[0]] = vertA.x;	tVs[triangleVerticleIndeces[1]] = vertA.y;	tVs[triangleVerticleIndeces[2]] = vertA.z;
		tVs[triangleVerticleIndeces[3]] = vertB.x;	tVs[triangleVerticleIndeces[4]] = vertB.y;	tVs[triangleVerticleIndeces[5]] = vertB.z;
		tVs[triangleVerticleIndeces[6]] = vertC.x;	tVs[triangleVerticleIndeces[7]] = vertC.y;	tVs[triangleVerticleIndeces[8]] = vertC.z;

		
	}

		//tVs[uint(volIndex1D/McFactor)] = 1.0f;

	//Get Corners from Edges


	//atomicAdd(tIDs[0],1);
	//tIDs[triIndex] = cubeIndex;
	

	
}


void main() {
	ivec3 index = ivec3(gl_GlobalInvocationID.xyz);

	//if we are not within the volume of interest -> return 
	int xCheck =  VolumeDims.x-McFactor;
	int yCheck =  VolumeDims.y-McFactor;
	int zCheck =  VolumeDims.z-McFactor;
	if ( index.x >= xCheck || index.y >= yCheck || index.z >= zCheck ) {
		return;
	}

	getIDs(index);
}