attribute vec3 pos;
attribute vec2 tex;
attribute vec3 nor;
attribute mat4 MVP;
attribute mat4 MIT;

varying vec2 texCoord;
varying vec3 normal;

void kore() {
	gl_Position = MVP * vec4(pos, 1.0);
	texCoord = tex;
	normal = (MIT * vec4(nor, 0.0)).xyz;
}