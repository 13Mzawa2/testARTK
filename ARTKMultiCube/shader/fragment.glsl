#version 330 core
 
// 頂点シェーダからの値を書き込みます
in vec4 vertexPosition_cameraspace;
in vec2 UV;
in vec3 Normal;
 
// アウトプットデータ
out vec3 color;
 
// すべてのメッシュで一定の値
uniform sampler2D myTextureSampler;		//	テクスチャサンプラー
uniform vec3 LightDirection;			//	光線方向
uniform vec3 LightColor;				//	光源色
 
void main()
{
	vec3 fnormal = normalize(Normal);					//	fragmentに渡されたNormalは正規化されていない
	vec3 halfway = normalize(LightDirection - vec3(vertexPosition_cameraspace));
	float cosine = max(dot(fnormal, halfway), 0);
	vec3 diffuse = LightColor * cosine;
	vec3 ambient = diffuse * vec3(0.1, 0.1, 0.1);

    // アウトプットカラー = 指定したUV座標のテクスチャの色にシェーディングを実行
    vec3 tempcolor = texture( myTextureSampler, UV ).rgb * diffuse + ambient;
	
	color = tempcolor;
}