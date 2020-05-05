#version 140
#extension GL_ARB_explicit_attrib_location : require

layout (location = 0) in vec3 aPos;

void main()
{
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
	//fourth coordinate is reserved for perspective division, w??
}
