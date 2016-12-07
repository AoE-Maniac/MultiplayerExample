#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D text;

varying vec2 texCoord;
varying vec3 normal;

void kore() {
	gl_FragColor = vec4(((texture2D(text, texCoord) * (0.8 + max(dot(vec3(0, 1, 0), normal), 0.0) * 0.2)).xyz), 1);
}