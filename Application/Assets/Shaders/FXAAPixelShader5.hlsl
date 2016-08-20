//FXAA - Min egna skrivna baserat på riktiga FXAA. 

Texture2D lumaTexture : register(t0);
SamplerState Sampler : register(s0);

struct PSIn
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4 PS_FXAA(PSIn _input) : SV_TARGET
{
	//We have Position
	//We have tex coordinates.
	int width = 0;
	int height = 0;
	lumaTexture.GetDimensions(width, height);
	
	float2 posM = _input.TexCoord;
	float4 rgbyM = lumaTexture.Sample(Sampler, posM);
	
	
	//fxaaQualityRcpFrame == Texelsize. Textures are represented by an array of texels.
	float2 fxaaQualityRcpFrame;
	fxaaQualityRcpFrame.x = 1.0f/(float)width;
	fxaaQualityRcpFrame.y = 1.0f/(float)height;
	
	//fxaaQualityEdgeThreshold - skulle kunna vara i constant buffern.
	/*
	// The minimum amount of local contrast required to apply algorithm.
    //   0.333 - too little (faster)
    //   0.250 - low quality
    //   0.166 - default
    //   0.125 - high quality 
    //   0.063 - overkill (slower)
	*/
	float fxxaQualityEdgeThresholdMax = 0.125;
	
	//fxaaQualityEdgeThresholdMin
	/*
	// Trims the algorithm from processing darks.
    //   0.0833 - upper limit (default, the start of visible unfiltered edges)
    //   0.0625 - high quality (faster)
    //   0.0312 - visible limit (slower)
    // Special notes when using FXAA_GREEN_AS_LUMA,
    //   Likely want to set this to zero.
    //   As colors that are mostly not-green
    //   will appear very dark in the green channel!
    //   Tune by looking at mostly non-green content,
    //   then start at zero and increase until aliasing is a problem.
	*/
	float fxaaQualityEdgeThresholdMin = 0.0625f;


	//We retrieve all 9 luminance pixels in the area of the pixel.
	//Middle, North, East, West, South, 
	float lumaM = lumaTexture.SampleLevel(Sampler, posM, 0.0f).w; //lumaTexture.Load(int3(pixelPos, 0)).w;
	float lumaN = lumaTexture.SampleLevel(Sampler, posM, 0.0f, int2(0, -1)).w;
	float lumaW = lumaTexture.SampleLevel(Sampler, posM, 0.0f, int2(-1, 0)).w;
	
	/*
	//Calculate color Deltas
	float2 delta;

	delta.xy = abs(lumaM - float2(lumaW, lumaN));
	//if delta is is greater/equal than the threshold return true
	float2 threshold = float2(0.1f, 0.1f);
	float2 edges = step(threshold, delta.xy);
	//Discard if no edge
	
	if (dot(edges, float2(1.0, 1.0)) == 0.0)
		return float4(0,0,0,1);*/
	
	
	
	float lumaE = lumaTexture.SampleLevel(Sampler, posM, 0.0f, int2(1, 0)).w; //lumaTexture.Load(int3(pixelPos, 0), int2(1, 0)).w;
	 //lumaTexture.Load(int3(pixelPos, 0), int2(-1, 0)).w;
	float lumaS = lumaTexture.SampleLevel(Sampler, posM, 0.0f, int2(0, 1)).w; //lumaTexture.Load(int3(pixelPos, 0), int2(0, 1)).w;
	
	float fxaaQualityEdgeThreshold = 0.166f;
	
	// Edge detection
	// Determine the lowest and highest value of luminance in all 9 pixels.
    float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));
	float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
    float rangeMaxScaled = rangeMax * fxaaQualityEdgeThreshold;
    float range = rangeMax - rangeMin;
    float rangeMaxClamped = max(fxaaQualityEdgeThresholdMin, rangeMaxScaled);
    bool earlyExit = range < rangeMaxClamped;
	
	if(earlyExit)
	{
		return rgbyM;
	}
	
	
	float lumaNW = lumaTexture.SampleLevel(Sampler, posM, 0.0, int2(-1,-1)).w;
    float lumaSE = lumaTexture.SampleLevel(Sampler, posM, 0.0, int2( 1, 1)).w;
	float lumaNE = lumaTexture.SampleLevel(Sampler, posM, 0.0, int2( 1,-1)).w;
    float lumaSW = lumaTexture.SampleLevel(Sampler, posM, 0.0, int2(-1, 1)).w; 
	
	float subpixRcpRange = 1.0/range;
    //Vertical and horizontal edge test
	float edgeV1 = abs((-2.0 * lumaN) + lumaNW + lumaNE);
	float edgeV2 = abs((-2.0 * lumaM) + lumaW  + lumaE);
	float edgeV3 = abs((-2.0 * lumaS) + lumaSW + lumaSE);
	
	float edgeH1 = abs((-2.0 * lumaW) + lumaNW + lumaSW);
	float edgeH2 = abs((-2.0 * lumaM) + lumaN  + lumaS);
	float edgeH3 = abs((-2.0 * lumaE) + lumaNE + lumaSE);
	
	float edgeVert 		= edgeV1 + (2.0 * edgeV2) + edgeV3;
    float edgeHorz 	= edgeH1 + (2.0 * edgeH2) + edgeH3;
    bool horzSpan = edgeHorz >= edgeVert;
	
	//Subpixel calculations
	float subpixelPart1 = (2*(lumaN + lumaW + lumaE + lumaS) + (lumaNW + lumaSW + lumaNE + lumaSE)) / 12.0f; //weights
	float subpixelPart2 = abs(subpixelPart1 - lumaM); //range
	float subpixelPart3 = saturate(subpixelPart2/range); //clamp the vector to between 0-1.
	float subpixelPart4 = (-2.0*subpixelPart3)+ 3.0;
	float subpixelPart5 = subpixelPart4 * subpixelPart3 * subpixelPart3;
	float subpixH 		=  subpixelPart5 * 0.75; // fxaaQualitySubpix == 0.75
	

    if(!horzSpan) lumaN = lumaW;
    if(!horzSpan) lumaS = lumaE;
	float lengthSign = fxaaQualityRcpFrame.x; //ok
    if(horzSpan) lengthSign = fxaaQualityRcpFrame.y; //ok
    

    float gradientN = lumaN - lumaM;
    float gradientS = lumaS - lumaM;
    float lumaNN = lumaN + lumaM;
    float lumaSS = lumaS + lumaM;
    float gradient = max(abs(gradientN), abs(gradientS));
	bool pairN = abs(gradientN) >= abs(gradientS);
    if(pairN) lengthSign = -lengthSign;
    if(!pairN) lumaNN = lumaSS;

    float2 posB;
    posB.x = posM.x;
    posB.y = posM.y;
    float2 offNP;
    offNP.x = ( horzSpan) ? fxaaQualityRcpFrame.x 	: 0.0;
    offNP.y = ( horzSpan) ? 0.0 					: fxaaQualityRcpFrame.y;
    if( horzSpan) posB.y += lengthSign * 0.5;
    if(!horzSpan) posB.x += lengthSign * 0.5;

    float2 posN;
    posN.x = posB.x - offNP.x;
    posN.y = posB.y - offNP.y;
    float2 posP;
    posP.x = posB.x + offNP.x;
    posP.y = posB.y + offNP.y;
    
    float lumaEndN = lumaTexture.SampleLevel(Sampler, posN, 0.0).w;
    
    float lumaEndP = lumaTexture.SampleLevel(Sampler, posP, 0.0).w;

    float gradientScaled = gradient * 1.0/4.0;
    float lumaMM = lumaM - lumaNN * 0.5;
    bool lumaMLTZero = lumaMM < 0.0;
	
	lumaEndN -= lumaNN * 0.5;
	lumaEndP -= lumaNN * 0.5;
	bool doneN = abs(lumaEndN) >= gradientScaled;
	bool doneP = abs(lumaEndP) >= gradientScaled;
	if(!doneN) posN.x -= offNP.x;
	if(!doneN) posN.y -= offNP.y;
	bool doneNP = (!doneN) || (!doneP);
	if(!doneP) posP.x += offNP.x;
	if(!doneP) posP.y += offNP.y;

	int FXAA_SEARCH_STEPS = 15;
	for(int i=0; i< FXAA_SEARCH_STEPS; i++)
	{
		if(doneNP) 
		{
            if(!doneN) lumaEndN = lumaTexture.SampleLevel(Sampler, posN, 0.0).w;
            if(!doneP) lumaEndP = lumaTexture.SampleLevel(Sampler, posP, 0.0).w;
            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
            doneN = abs(lumaEndN) >= gradientScaled;
            doneP = abs(lumaEndP) >= gradientScaled;
            if(!doneN) posN.x -= offNP.x;
            if(!doneN) posN.y -= offNP.y;
            doneNP = (!doneN) || (!doneP);
            if(!doneP) posP.x += offNP.x;
            if(!doneP) posP.y += offNP.y;
		}
	}
		
    float dstP = posP.x - posM.x;
	float dstN = posM.x - posN.x;
    if(!horzSpan) dstN = posM.y - posN.y;
    if(!horzSpan) dstP = posP.y - posM.y;

    bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
    float spanLength = (dstP + dstN);
    bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
    float spanLengthRcp = 1.0/spanLength;

    bool directionN = dstN < dstP;
    float dst = min(dstN, dstP);
    bool goodSpan = directionN ? goodSpanN : goodSpanP;
    
    float pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
   
    float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
    float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
    if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
    if( horzSpan) posM.y += pixelOffsetSubpix * lengthSign;
	
	return lumaTexture.SampleLevel(Sampler, posM, 0.0);
}