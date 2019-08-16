uniform highp mat4 transformationProjectionMatrix;

layout(location = 0) in highp vec4 position;

#ifdef TEXTURED
layout(location = 1) in mediump vec2 textureCoordinates;
out mediump vec2 interpolatedTextureCoordinates;
#endif

#ifdef VERTEX_COLORED
layout(location = 1) in mediump vec3 color;
#endif

out mediump vec3 v_color;

#ifdef PER_VERTEX_IDS
flat out uint v_objectId;
#endif

void main() {
  gl_Position = transformationProjectionMatrix * vec4(position.xyz, 1.0);

  #ifdef TEXTURED
  interpolatedTextureCoordinates = textureCoordinates;
  #endif

  #ifdef VERTEX_COLORED
  v_color = color;
  #endif
  #ifdef PER_VERTEX_IDS
  v_objectId = uint(position.w);
  #endif
}
