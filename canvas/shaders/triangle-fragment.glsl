#version 330
uniform float scale;
out vec4 outputColor;
smooth in vec3 color_to_fragment;
smooth in float striper_to_fragment;
smooth in float alpha_to_fragment;
smooth in vec2 round_pos_to_fragment;
flat in float line_length_to_fragment;
flat in float line_height_px_to_fragment;
flat in int flags_to_fragment;

void main() {
  float alpha = alpha_to_fragment;
  bool disc = false;
  if(mod(striper_to_fragment,20)>2 || (flags_to_fragment&2)!=0) {
    disc = true;
  }
  if(abs(round_pos_to_fragment.x)>(line_length_to_fragment/2)) {
    if(length(abs(round_pos_to_fragment)-vec2(line_length_to_fragment/2,0))>1) {
      disc = true;
    }
    else {
      if((1-length(abs(round_pos_to_fragment)-vec2(line_length_to_fragment/2,0)))*line_height_px_to_fragment<1) {
        disc = false;
        alpha = 1;
      }
    }
  }
  else {
    if((1-abs(round_pos_to_fragment.y))*line_height_px_to_fragment<(1)) {
      disc = false;
      alpha = 1;
    }
  }
  if(disc)
    discard;
  outputColor = vec4(color_to_fragment, alpha);
}
