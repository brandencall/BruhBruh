#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform float damage;

void main()
{
    vec4 color = texture(texture0, fragTexCoord);

    vec2 uv = fragTexCoord - vec2(0.5);
    float dist = length(uv);

    float vignette = smoothstep(0.3, 0.8, dist);

    color.rgb = mix(
        color.rgb,
        vec3(0.5, 0.0, 0.0),
        vignette * damage
    );

    finalColor = color;
}
