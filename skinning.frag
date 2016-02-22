#version 330 core

#define M_PI 3.1415926535897932384626433832795
#define EPS 0.1

const vec3 phongDiffuse=vec3(1.0);
const vec3 Ks=vec3(1.0,0.5,0.0);
const float phongShininess=3000.0;
const vec3 lightValue=vec3(0.8), lightAmbient=vec3(0.1);
const float gloss=0.9; 
 
in vec2 TexCoord0;
in vec3 Normal0;                                                                   
in vec3 lightVec;
in vec3 eyeVec;
in vec4 shadowCoord;

uniform sampler2D shadowMap;
uniform sampler2D texture0;
uniform int shadows;

out vec4 FragColor;

//declaration of functions
vec3 SpecularBHDR(vec3 L, vec3 H, vec3 N,vec3 Ks,float a);
float saturate(float a);
vec3 Ilumination(vec3 L,vec3 H,vec3 N,vec3 Ks,float a,vec3 Kd);
bool InShadow();

void main()
{
	vec3 L = normalize(lightVec);
	vec3 N = normalize(Normal0);
	vec3 E = normalize(eyeVec);  
	vec3 H=normalize(L+E);
	
	float a=pow(phongShininess,gloss);
    vec4 texture= texture2D(texture0, TexCoord0.xy);  
	
	if (shadows==1 && InShadow()){		//is not working
		FragColor.rgb=texture.rgb*lightAmbient;
	}else{
		FragColor.rgb=Ilumination(L,H,N,Ks,a,texture.rgb);
	}
	
	FragColor.a=1.0;
}


float saturate(float a){
	return max(a,0.0);
}

vec3 Ilumination(vec3 L,vec3 H,vec3 N,vec3 Ks,float a,vec3 Kd){
	float NdotL=dot(N,L);
	vec3 spec=SpecularBHDR(L,H,N,Ks,a);
	vec3 illum=lightAmbient*Kd+( saturate(NdotL)*Kd + spec)*lightValue;
	return vec3(illum);
}

vec3 SpecularBHDR(vec3 L, vec3 H, vec3 N,vec3 Ks,float a){
	float LdotH=dot(L,H);
	float NdotH=dot(N,H);

	vec3 fressnel=Ks + (1.0-Ks)*pow(1.0-LdotH,5.0);
	float d=(a+2.0)*pow(saturate(NdotH),a)/(2.0*M_PI);
	float g=1.0/pow(saturate(LdotH),2.0);
	return fressnel*g*d/4.0; //I am using the approximation of G, therefore I am not dividing by the other dot products
}

bool InShadow(){
	if (shadowCoord.w>0){		
		vec2 shadowIndex=(shadowCoord.xy)/shadowCoord.w;		//shadow coordinates
		if (shadowIndex.s>=0 && shadowIndex.s<=1 && shadowIndex.t>=0 && shadowIndex.t<=1){	//if they are in the correct range
			float lightDepth = texture(shadowMap,shadowIndex).w;		//get the depth from the shadow image
			float pixelDepth = shadowCoord.w;
			if ((pixelDepth-EPS>lightDepth)){			//compare depth to decide if the pixel is in shadow
				return true;
			}
		}
	}
	return false;
}


