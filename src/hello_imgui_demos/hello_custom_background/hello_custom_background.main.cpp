#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "imgui.h"

#include <unordered_map>


/******************************************************************************
 *
 * Uniforms Utilities
 *
******************************************************************************/

struct MyVec3
{
    float x, y, z;
};


// Helper struct for checking if a type is supported (typical C++ shenanigans)
template<class T> struct UnsupportedType : std::false_type {};

template<typename T>
void ApplyUniform(GLint location, const T& value)
{
    if constexpr (std::is_same<T, int>::value)
        glUniform1i(location, value);
    else if constexpr (std::is_same<T, float>::value)
        glUniform1f(location, value);
    else if constexpr (std::is_same<T, MyVec3>::value)
        glUniform3fv(location, 1, &value.x);
    else if constexpr (std::is_same<T, ImVec2>::value)
        glUniform2fv(location, 1, &value.x);
    else if constexpr (std::is_same<T, ImVec4>::value)
        glUniform4fv(location, 1, &value.x);
    else
        static_assert(UnsupportedType<T>::value, "Unsupported type");
}


struct IUniform
{
    GLint location = 0;

    void StoreLocation(GLuint shaderProgram, const std::string& name)
    {
        location = glGetUniformLocation(shaderProgram, name.c_str());
    }

    virtual ~IUniform() {}

    virtual void Apply() = 0;
};


template<typename T>
struct Uniform : public IUniform
{
    T value;
    Uniform(const T& initialValue): IUniform(), value(initialValue) {}
    void Apply() override { ApplyUniform(location, value); }
};


struct UniformsList
{
    std::unordered_map<std::string, std::unique_ptr<IUniform>> Uniforms;

    template<typename T> void AddUniform(const std::string& name, const T& initialValue)
    {
        Uniforms[name] = std::make_unique<Uniform<T>>(initialValue);
    }

    void StoreUniformLocations(GLuint shaderProgram)
    {
        for (auto& uniform : Uniforms)
            uniform.second->StoreLocation(shaderProgram, uniform.first);
    }

    void ApplyUniforms()
    {
        for (auto& uniform : Uniforms)
            uniform.second->Apply();
    }

    template<typename T> T& GetUniformValue(const std::string& name)
    {
        IM_ASSERT(Uniforms.find(name) != Uniforms.end());
        auto& uniform = Uniforms[name];
        Uniform<T>* asT = dynamic_cast<Uniform<T>*>(uniform.get());
        IM_ASSERT(asT != nullptr);
        return asT->value;
    }

    template<typename T> void SetUniformValue(const std::string& name, const T& value)
    {
        IM_ASSERT(Uniforms.find(name) != Uniforms.end());
        auto& uniform = Uniforms[name];
        Uniform<T>* asT = dynamic_cast<Uniform<T>*>(uniform.get());
        IM_ASSERT(asT != nullptr);
        asT->value = value;
    }
};


/******************************************************************************
 *
 * Shader Utilities
 *
******************************************************************************/

void FailOnGlLinkError(GLuint shaderProgram)
{
    GLint isLinked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
    if (!isLinked) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        IM_ASSERT(isLinked);
    }
}

void FailOnShaderError(GLuint shader)
{
    GLint shaderCompileSuccess;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileSuccess);
    if (!shaderCompileSuccess)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        IM_ASSERT(shaderCompileSuccess);
    }
}

void FailOnOpenGlError()
{
    int countOpenGlError = 0;
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error: " << err << std::endl;
        ++countOpenGlError;
    }
    IM_ASSERT(countOpenGlError == 0);
}

GLuint CompileShader(GLuint type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    FailOnShaderError(shader); // Check for compilation errors
    return shader;
}

GLuint CreateFullScreenQuadVAO()
{
    // Define the vertex data for a full-screen quad
    float vertices[] = {
        // positions   // texCoords
        -1.0f, -1.0f,  0.0f, 0.0f, // bottom left  (0)
        1.0f, -1.0f,  1.0f, 0.0f, // bottom right (1)
        -1.0f,  1.0f,  0.0f, 1.0f, // top left     (2)
        1.0f,  1.0f,  1.0f, 1.0f  // top right    (3)
    };
    // Generate and bind the VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Generate and bind the VBO
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Fill the VBO with vertex data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind the VAO (and VBO)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vao;
}

GLuint CreateShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    FailOnGlLinkError(shaderProgram);

    // Delete shader objects once linked
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Check for any OpenGL errors
    FailOnOpenGlError();

    return shaderProgram;
}


/******************************************************************************
 *
 * Shader code
 *
******************************************************************************/

const char* GVertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";


// See https://www.shadertoy.com/view/Ms2SD1 / Many thanks to Alexander Alekseev aka TDM
const char* GFragmentShaderSource = R"(
#version 330 core
/*
 * "Seascape" by Alexander Alekseev aka TDM - 2014
 * License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
 * Contact: tdmaav@gmail.com
 */


in vec2 TexCoord;
out vec4 FragColor;

uniform vec2 iResolution;  // Window resolution
uniform float iTime;      // Shader elapsed time
uniform vec2 iMouse;      // Mouse position


const int NUM_STEPS = 8;
const float PI	 	= 3.141592;
const float EPSILON	= 1e-3;
#define EPSILON_NRM (0.1 / iResolution.x)

// sea
//const int ITER_GEOMETRY = 3;
//const int ITER_FRAGMENT = 5;
//const float SEA_HEIGHT = 0.6;
//const float SEA_CHOPPY = 4.0;
//const float SEA_SPEED = 0.8;
//const float SEA_FREQ = 0.16;
//const vec3 SEA_WATER_COLOR = vec3(0.8,0.9,0.6)*0.6;
//const vec3 SEA_BASE = vec3(0.0,0.09,0.18);

uniform int ITER_GEOMETRY;
uniform int ITER_FRAGMENT;
uniform float SEA_HEIGHT;
uniform float SEA_CHOPPY;
uniform float SEA_SPEED;
uniform float SEA_FREQ;
uniform vec3 SEA_WATER_COLOR;
uniform vec3 SEA_BASE;

#define SEA_TIME (1.0 + iTime * SEA_SPEED)
const mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);

// math
mat3 fromEuler(vec3 ang) {
	vec2 a1 = vec2(sin(ang.x),cos(ang.x));
    vec2 a2 = vec2(sin(ang.y),cos(ang.y));
    vec2 a3 = vec2(sin(ang.z),cos(ang.z));
    mat3 m;
    m[0] = vec3(a1.y*a3.y+a1.x*a2.x*a3.x,a1.y*a2.x*a3.x+a3.y*a1.x,-a2.y*a3.x);
	m[1] = vec3(-a2.y*a1.x,a1.y*a2.y,a2.x);
	m[2] = vec3(a3.y*a1.x*a2.x+a1.y*a3.x,a1.x*a3.x-a1.y*a3.y*a2.x,a2.y*a3.y);
	return m;
}
float hash( vec2 p ) {
	float h = dot(p,vec2(127.1,311.7));
    return fract(sin(h)*43758.5453123);
}
float noise( in vec2 p ) {
    vec2 i = floor( p );
    vec2 f = fract( p );
	vec2 u = f*f*(3.0-2.0*f);
    return -1.0+2.0*mix( mix( hash( i + vec2(0.0,0.0) ),
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ),
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}

// lighting
float diffuse(vec3 n,vec3 l,float p) {
    return pow(dot(n,l) * 0.4 + 0.6,p);
}
float specular(vec3 n,vec3 l,vec3 e,float s) {
    float nrm = (s + 8.0) / (PI * 8.0);
    return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

// sky
vec3 getSkyColor(vec3 e) {
    e.y = (max(e.y,0.0)*0.8+0.2)*0.8;
    return vec3(pow(1.0-e.y,2.0), 1.0-e.y, 0.6+(1.0-e.y)*0.4) * 1.1;
}

// sea
float sea_octave(vec2 uv, float choppy) {
    uv += noise(uv);
    vec2 wv = 1.0-abs(sin(uv));
    vec2 swv = abs(cos(uv));
    wv = mix(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

float map(vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;

    float d, h = 0.0;
    for(int i = 0; i < ITER_GEOMETRY; i++) {
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
    	d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;
    	uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

float map_detailed(vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;

    float d, h = 0.0;
    for(int i = 0; i < ITER_FRAGMENT; i++) {
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
    	d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;
    	uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, vec3 dist) {
    float fresnel = clamp(1.0 - dot(n,-eye), 0.0, 1.0);
    fresnel = min(pow(fresnel,3.0), 0.5);

    vec3 reflected = getSkyColor(reflect(eye,n));
    vec3 refracted = SEA_BASE + diffuse(n,l,80.0) * SEA_WATER_COLOR * 0.12;

    vec3 color = mix(refracted,reflected,fresnel);

    float atten = max(1.0 - dot(dist,dist) * 0.001, 0.0);
    color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;

    color += vec3(specular(n,l,eye,60.0));

    return color;
}

// tracing
vec3 getNormal(vec3 p, float eps) {
    vec3 n;
    n.y = map_detailed(p);
    n.x = map_detailed(vec3(p.x+eps,p.y,p.z)) - n.y;
    n.z = map_detailed(vec3(p.x,p.y,p.z+eps)) - n.y;
    n.y = eps;
    return normalize(n);
}

float heightMapTracing(vec3 ori, vec3 dir, out vec3 p) {
    float tm = 0.0;
    float tx = 1000.0;
    float hx = map(ori + dir * tx);
    if(hx > 0.0) {
        p = ori + dir * tx;
        return tx;
    }
    float hm = map(ori + dir * tm);
    float tmid = 0.0;
    for(int i = 0; i < NUM_STEPS; i++) {
        tmid = mix(tm,tx, hm/(hm-hx));
        p = ori + dir * tmid;
    	float hmid = map(p);
		if(hmid < 0.0) {
        	tx = tmid;
            hx = hmid;
        } else {
            tm = tmid;
            hm = hmid;
        }
    }
    return tmid;
}

vec3 getPixel(in vec2 coord, float time) {
    vec2 uv = coord / iResolution.xy;
    uv = uv * 2.0 - 1.0;
    uv.x *= iResolution.x / iResolution.y;

    // ray
    vec3 ang = vec3(sin(time*3.0)*0.1,sin(time)*0.2+0.3,time);
    vec3 ori = vec3(0.0,3.5,time*5.0);
    vec3 dir = normalize(vec3(uv.xy,-2.0)); dir.z += length(uv) * 0.14;
    dir = normalize(dir) * fromEuler(ang);

    // tracing
    vec3 p;
    heightMapTracing(ori,dir,p);
    vec3 dist = p - ori;
    vec3 n = getNormal(p, dot(dist,dist) * EPSILON_NRM);
    vec3 light = normalize(vec3(0.0,1.0,0.8));

    // color
    return mix(
        getSkyColor(dir),
        getSeaColor(p,n,light,dir,dist),
    	pow(smoothstep(0.0,-0.02,dir.y),0.2));
}

// #define AA
void main()
{
    vec2 fragCoord = TexCoord * iResolution;
    float time = iTime * 0.3 + iMouse.x * 0.01;

    // Compute uv based on fragCoord
    //    vec2 uv = fragCoord / iResolution.xy;
    //    uv = uv * 2.0 - 1.0;
    //    uv.x *= iResolution.x / iResolution.y;

#ifdef AA
    vec3 color = vec3(0.0);
    // Anti-aliasing loop (optional, can be removed for performance)
    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            vec2 uv = fragCoord + vec2(i, j) / 3.0;
            color += getPixel(uv, time);
        }
    }
    color /= 9.0;
#else
    vec3 color = getPixel(fragCoord, time);
#endif

    // Post-processing (adjust as needed)
    FragColor = vec4(pow(color, vec3(0.65)), 1.0);
}
)";


/******************************************************************************
 *
 * Our App starts here
 *
******************************************************************************/

struct AppState
{
    UniformsList Uniforms;
    GLuint ShaderProgram;
    GLuint FullScreenQuadVAO;

    AppState()
    {
        Uniforms.AddUniform("ITER_GEOMETRY", 3);
        Uniforms.AddUniform("ITER_FRAGMENT", 5);
        Uniforms.AddUniform("SEA_HEIGHT", 0.6f);
        Uniforms.AddUniform("SEA_CHOPPY", 4.0f);
        Uniforms.AddUniform("SEA_SPEED", 0.8f);
        Uniforms.AddUniform("SEA_FREQ", 0.16f);
        Uniforms.AddUniform("SEA_WATER_COLOR", MyVec3{0.8f * 0.6f, 0.9f * 0.6f, 0.6f * 0.6f});
        Uniforms.AddUniform("SEA_BASE", MyVec3{0.0f, 0.09f, 0.18f});

        Uniforms.AddUniform("iResolution", ImVec2{100.f, 100.f});
        Uniforms.AddUniform("iTime", 0.f);
        Uniforms.AddUniform("iMouse", ImVec2{0.f, 0.f});
    }

    void ApplyUniforms() { Uniforms.ApplyUniforms(); }
    void StoreUniformLocations() { Uniforms.StoreUniformLocations(ShaderProgram); }
};


void InitAppResources3D(AppState& appState)
{
    appState.ShaderProgram = CreateShaderProgram(GVertexShaderSource, GFragmentShaderSource);
    appState.FullScreenQuadVAO = CreateFullScreenQuadVAO();
    appState.StoreUniformLocations();
}


void DeInitAppResources3D(AppState& appState)
{
    glDeleteProgram(appState.ShaderProgram);
    glDeleteVertexArrays(1, &appState.FullScreenQuadVAO);
}


ImVec2 ScaledDisplaySize()
{
    auto& io = ImGui::GetIO();
    auto r = ImVec2(io.DisplaySize.x * io.DisplayFramebufferScale.x,
                    io.DisplaySize.y * io.DisplayFramebufferScale.y);
    return r;
}


// Our custom background callback: it display the sea shader
void CustomBackground(AppState& appState)
{
    ImVec2 displaySize = ScaledDisplaySize();
    glViewport(0, 0, (GLsizei)displaySize.x, (GLsizei)displaySize.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(appState.ShaderProgram);

    appState.Uniforms.SetUniformValue("iResolution", displaySize);
    appState.Uniforms.SetUniformValue("iTime", (float)ImGui::GetTime());

    // Optional: Set the iMouse uniform if you use it
    // appState.Uniforms.SetUniformValue("iMouse", ImGui::IsMouseDown(0) ? ImGui::GetMousePos() : ImVec2(0.f, 0.f));
    appState.Uniforms.SetUniformValue("iMouse", ImVec2(0.f, 0.f));

    appState.ApplyUniforms();

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(appState.FullScreenQuadVAO); // Render a full-screen quad (Bind the VAO)
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Draw the quad
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0); // Unbind the VAO
    glUseProgram(0); // Unbind the shader program
}


void Gui(AppState& appState)
{
    ImGui::SetNextWindowPos(HelloImGui::EmToVec2(0.f, 0.f), ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(HelloImGui::EmToVec2(30.f, 17.f), ImGuiCond_Appearing);
    ImGui::Begin("Shader parameters");

    ImGui::Text("Shader: \"Seascape\" by Alexander Alekseev aka TDM - 2014\n");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip(
                "License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.\n"
                "Contact: tdmaav@gmail.com\n\n"
                "https://www.shadertoy.com/view/Ms2SD1"
                );

    auto& uniforms = appState.Uniforms;
    ImGui::SliderFloat("SEA_HEIGHT", &uniforms.GetUniformValue<float>("SEA_HEIGHT"), 0.1f, 2.1f);
    ImGui::SliderFloat("SEA_CHOPPY", &uniforms.GetUniformValue<float>("SEA_CHOPPY"), 0.1f, 10.0f);

    //ImGui::ColorEdit3("SEA_WATER_COLOR", &uniforms.GetUniformValue<MyVec3>("SEA_WATER_COLOR").x);
    ImGui::ColorEdit3("SEA_BASE", &uniforms.GetUniformValue<MyVec3>("SEA_BASE").x);

    ImGui::SliderFloat("SEA_SPEED", &uniforms.GetUniformValue<float>("SEA_SPEED"), 0.1f, 3.0f);
    ImGui::SliderFloat("SEA_FREQ", &uniforms.GetUniformValue<float>("SEA_FREQ"), 0.01f, 0.5f);
    ImGui::SliderInt("ITER_GEOMETRY", &uniforms.GetUniformValue<int>("ITER_GEOMETRY"), 1, 5);
    ImGui::SliderInt("ITER_FRAGMENT", &uniforms.GetUniformValue<int>("ITER_FRAGMENT"), 3, 10);

    ImGui::Text("FPS: %.1f", HelloImGui::FrameRate());

    ImGui::End();
}


int main(int , char *[])
{
    HelloImGui::RunnerParams params;
    params.fpsIdling.enableIdling = false;
    params.appWindowParams.windowGeometry.size = {1200, 720};
    params.appWindowParams.windowTitle = "Hello ImGui: custom 3D background - shader by Alexander Alekseev aka TDM - 2014";
    params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::NoDefaultWindow;

    AppState appState;
    params.callbacks.PostInit = [&appState]() { InitAppResources3D(appState); };
    params.callbacks.ShowGui = [&appState]() { Gui(appState); };
    params.callbacks.CustomBackground = [&appState]() { CustomBackground(appState); };
    params.callbacks.BeforeExit = [&appState]() { DeInitAppResources3D(appState); };
    HelloImGui::Run(params);
    return 0;
}
