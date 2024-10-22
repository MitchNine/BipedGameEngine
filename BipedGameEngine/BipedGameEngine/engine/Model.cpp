#include "Model.h"
using namespace bpd;
using namespace DirectX;

Model::Model() {
	transform.MTranslation	= DirectX::XMMATRIX();
	transform.MRotation		= DirectX::XMMATRIX();
	transform.MScale			= DirectX::XMMATRIX();

	transform.scail			= XMFLOAT3(1.0f,1.0f,1.0f);
	transform.position		= XMFLOAT3(0.0f,0.0f,0.0f);
	transform.rotation		= XMFLOAT3(0.0f,0.0f,0.0f);
}
Model::~Model() {}

HRESULT Model::CreateTexture(ID3D11Device* device,const WCHAR* filename,ID3D11ShaderResourceView **texture) {
	wchar_t ext[_MAX_EXT];
	_wsplitpath_s(filename,nullptr,0,nullptr,0,nullptr,0,ext,_MAX_EXT);

	DirectX::ScratchImage image;
	HRESULT hr;
	if(_wcsicmp(ext,L".dds") == 0) {
		hr = LoadFromDDSFile(filename,DirectX::DDS_FLAGS_NONE,nullptr,image);
	} else if(_wcsicmp(ext,L".tga") == 0) {
		hr = LoadFromTGAFile(filename,nullptr,image);
	} else if(_wcsicmp(ext,L".hdr") == 0) {
		hr = LoadFromHDRFile(filename,nullptr,image);
	} else {
		hr = LoadFromWICFile(filename,DirectX::WIC_FLAGS_NONE,nullptr,image);
	}
	if(SUCCEEDED(hr)) {
		ID3D11ShaderResourceView* * pSRV = nullptr;
		hr = CreateShaderResourceView(device,
			image.GetImages(),image.GetImageCount(),
			image.GetMetadata(),texture);
	}

	return hr;
}

bool Model::LoadObjModel(
	ID3D11Device* device,		// Pointer to the device
	std::wstring path,			// object path
	std::wstring Filename,      // obj m_model filename (m_model.obj)
	IDXGISwapChain* SwapChain,	// Pointer to the SwapChain
	bool IsRHCoordSys,          // True if m_model was created in right hand coord system
	bool ComputeNormals			// True to compute the normals, false to use the files normals
) {
	HRESULT hr = 0;

	std::wstring setpath = path;
	std::wifstream fileIn(setpath.append(Filename).c_str());   // Open file
	setpath.clear();

	std::wstring meshMatLib; // String to hold our obj material library filename (m_model.mtl)

							 // Arrays to store our m_model's information
	std::vector<DWORD> indices;
	std::vector<XMFLOAT3> vertPos;
	std::vector<XMFLOAT3> vertNorm;
	std::vector<XMFLOAT2> vertTexCoord;
	std::vector<std::wstring> meshMaterials;

	// Vertex definition indices
	std::vector<int> vertPosIndex;
	std::vector<int> vertNormIndex;
	std::vector<int> vertTCIndex;

	// Make sure we have a default if no tex coords or normals are defined
	bool hasTexCoord = false;
	bool hasNorm = false;

	// Temp variables to store into vectors
	std::wstring meshMaterialsTemp;
	int vertPosIndexTemp;
	int vertNormIndexTemp;
	int vertTCIndexTemp;

	wchar_t checkChar;      // The variable we will use to store one char from file at a time
	std::wstring face;      // Holds the string containing our face vertices
	int vIndex = 0;         // Keep track of our vertex index count
	int triangleCount = 0;  // Total Triangles
	int totalVerts = 0;
	int meshTriangles = 0;

	model.Center = DirectX::XMFLOAT3(0, 0, 0);

	//Check to see if the file was opened
	if(fileIn) {
		while(fileIn) {
			checkChar = fileIn.get();   //Get next char

			switch(checkChar) {
				// A comment. Skip rest of the line
			case '#':
				checkChar = fileIn.get();
				while(checkChar != '\n')
					checkChar = fileIn.get();
				break;

				// Get Vertex Descriptions;
			case 'v':
				checkChar = fileIn.get();
				if(checkChar == ' ')  // v - vert position
				{
					float vz,vy,vx;
					fileIn >> vx >> vy >> vz;   // Store the next three types

					if(IsRHCoordSys)            // If m_model is from an RH Coord System
						vertPos.push_back(XMFLOAT3(vx,vy,vz * -1.0f));   // Invert the Z axis
					else
						vertPos.push_back(XMFLOAT3(vx,vy,vz));
				}
				if(checkChar == 't')  // vt - vert tex coords
				{
					float vtcu,vtcv;
					fileIn >> vtcu >> vtcv;     // Store next two types

					if(IsRHCoordSys)            // If m_model is from an RH Coord System
						vertTexCoord.push_back(XMFLOAT2(vtcu,1.0f - vtcv));  // Reverse the "v" axis
					else
						vertTexCoord.push_back(XMFLOAT2(vtcu,vtcv));

					hasTexCoord = true;         // We know the m_model uses texture coords
				}
				if(checkChar == 'n')          // vn - vert normal
				{
					float vnx,vny,vnz;
					fileIn >> vnx >> vny >> vnz;    // Store next three types

					if(IsRHCoordSys)                // If m_model is from an RH Coord System
						vertNorm.push_back(XMFLOAT3(vnx,vny,vnz * -1.0f));  // Invert the Z axis
					else
						vertNorm.push_back(XMFLOAT3(vnx,vny,vnz));

					hasNorm = true;                 // We know the m_model defines normals
				}
				break;

				// New group (Subset)
			case 'o': // g - defines a group
				checkChar = fileIn.get();
				if(checkChar == ' ') {
					model.SubsetIndexStart.push_back(vIndex);       // Start index for this subset
					model.Subsets++;
				}
				break;

				// Get Face Index
			case 'f': // f - defines the faces
				checkChar = fileIn.get();
				if(checkChar == ' ') {
					face = L"";               // Holds all vertex definitions for the face (eg. "1\1\1 2\2\2 3\3\3")
					std::wstring VertDef;   // Holds one vertex definition at a time (eg. "1\1\1")
					triangleCount = 0;      // Keep track of the triangles for this face, since we will have to 
											// "retriangulate" faces with more than 3 sides

					checkChar = fileIn.get();
					while(checkChar != '\n') {
						face += checkChar;          // Add the char to our face string
						checkChar = fileIn.get();   // Get the next Character
						if(checkChar == ' ')      // If its a space...
							triangleCount++;        // Increase our triangle count
					}

					// Check for space at the end of our face string
					if(face[face.length() - 1] == ' ')
						triangleCount--;    // Each space adds to our triangle count

					triangleCount -= 1;     // Ever vertex in the face AFTER the first two are new faces

					std::wstringstream ss(face);

					if(face.length() > 0) {
						int firstVIndex,lastVIndex;    // Holds the first and last vertice's index

						for(int i = 0; i < 3; ++i)      // First three vertices (first triangle)
						{
							ss >> VertDef;  // Get vertex definition (vPos/vTexCoord/vNorm)

							std::wstring vertPart;
							int whichPart = 0;      // (vPos, vTexCoord, or vNorm)

													// Parse this string
							for(int j = 0; j < VertDef.length(); ++j) {
								if(VertDef[j] != '/') // If there is no divider "/", add a char to our vertPart
									vertPart += VertDef[j];

								// If the current char is a divider "/", or its the last character in the string
								if(VertDef[j] == '/' || j == VertDef.length() - 1) {
									std::wistringstream wstringToInt(vertPart); // Used to convert wstring to int

									if(whichPart == 0)  // If it's the vPos
									{
										wstringToInt >> vertPosIndexTemp;
										vertPosIndexTemp -= 1;      // subtract one since c++ arrays start with 0, and obj start with 1

																	// Check to see if the vert pos was the only thing specified
										if(j == VertDef.length() - 1) {
											vertNormIndexTemp = 0;
											vertTCIndexTemp = 0;
										}
									}

									else if(whichPart == 1) // If vTexCoord
									{
										if(vertPart != L"")   // Check to see if there even is a tex coord
										{
											wstringToInt >> vertTCIndexTemp;
											vertTCIndexTemp -= 1;   // subtract one since c++ arrays start with 0, and obj start with 1
										} else    // If there is no tex coord, make a default
											vertTCIndexTemp = 0;

										// If the cur. char is the second to last in the string, then
										// there must be no normal, so set a default normal
										if(j == VertDef.length() - 1)
											vertNormIndexTemp = 0;

									} else if(whichPart == 2) // If vNorm
									{
										std::wistringstream wstringToInt(vertPart);

										wstringToInt >> vertNormIndexTemp;
										vertNormIndexTemp -= 1;     // subtract one since c++ arrays start with 0, and obj start with 1
									}

									vertPart = L"";   // Get ready for next vertex part
									whichPart++;    // Move on to next vertex part                  
								}
							}

							// Check to make sure there is at least one subset
							if(model.Subsets == 0) {
								model.SubsetIndexStart.push_back(vIndex);       // Start index for this subset
								model.Subsets++;
							}

							// Avoid duplicate vertices
							bool vertAlreadyExists = false;
							if(totalVerts >= 3) // Make sure we at least have one triangle to check
							{
								// Loop through all the vertices
								for(int iCheck = 0; iCheck < totalVerts; ++iCheck) {
									// If the vertex position and texture coordinate in memory are the same
									// As the vertex position and texture coordinate we just now got out
									// of the obj file, we will set this faces vertex index to the vertex's
									// index value in memory. This makes sure we don't create duplicate vertices
									if(vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists) {
										if(vertTCIndexTemp == vertTCIndex[iCheck]) {
											model.Indices.push_back(iCheck);        // Set index for this vertex
											vertAlreadyExists = true;       // If we've made it here, the vertex already exists
										}
									}
								}
							}

							// If this vertex is not already in our vertex arrays, put it there
							if(!vertAlreadyExists) {
								vertPosIndex.push_back(vertPosIndexTemp);
								vertTCIndex.push_back(vertTCIndexTemp);
								vertNormIndex.push_back(vertNormIndexTemp);
								totalVerts++;   // We created a new vertex
								model.Indices.push_back(totalVerts - 1);  // Set index for this vertex
							}

							// If this is the very first vertex in the face, we need to
							// make sure the rest of the triangles use this vertex
							if(i == 0) {
								firstVIndex = model.Indices[vIndex];    //The first vertex index of this FACE

							}

							// If this was the last vertex in the first triangle, we will make sure
							// the next triangle uses this one (eg. tri1(1,2,3) tri2(1,3,4) tri3(1,4,5))
							if(i == 2) {
								lastVIndex = model.Indices[vIndex]; // The last vertex index of this TRIANGLE
							}
							vIndex++;   // Increment index count
						}

						meshTriangles++;    // One triangle down

											// If there are more than three vertices in the face definition, we need to make sure
											// we convert the face to triangles. We created our first triangle above, now we will
											// create a new triangle for every new vertex in the face, using the very first vertex
											// of the face, and the last vertex from the triangle before the current triangle
						for(int l = 0; l < triangleCount - 1; ++l)    // Loop through the next vertices to create new triangles
						{
							// First vertex of this triangle (the very first vertex of the face too)
							model.Indices.push_back(firstVIndex);           // Set index for this vertex
							vIndex++;

							// Second Vertex of this triangle (the last vertex used in the tri before this one)
							model.Indices.push_back(lastVIndex);            // Set index for this vertex
							vIndex++;

							// Get the third vertex for this triangle
							ss >> VertDef;

							std::wstring vertPart;
							int whichPart = 0;

							// Parse this string (same as above)
							for(int j = 0; j < VertDef.length(); ++j) {
								if(VertDef[j] != '/')
									vertPart += VertDef[j];
								if(VertDef[j] == '/' || j == VertDef.length() - 1) {
									std::wistringstream wstringToInt(vertPart);

									if(whichPart == 0) {
										wstringToInt >> vertPosIndexTemp;
										vertPosIndexTemp -= 1;

										// Check to see if the vert pos was the only thing specified
										if(j == VertDef.length() - 1) {
											vertTCIndexTemp = 0;
											vertNormIndexTemp = 0;
										}
									} else if(whichPart == 1) {
										if(vertPart != L"") {
											wstringToInt >> vertTCIndexTemp;
											vertTCIndexTemp -= 1;
										} else
											vertTCIndexTemp = 0;
										if(j == VertDef.length() - 1)
											vertNormIndexTemp = 0;

									} else if(whichPart == 2) {
										std::wistringstream wstringToInt(vertPart);

										wstringToInt >> vertNormIndexTemp;
										vertNormIndexTemp -= 1;
									}

									vertPart = L"";
									whichPart++;
								}
							}

							// Check for duplicate vertices
							bool vertAlreadyExists = false;
							if(totalVerts >= 3) // Make sure we at least have one triangle to check
							{
								for(int iCheck = 0; iCheck < totalVerts; ++iCheck) {
									if(vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists) {
										if(vertTCIndexTemp == vertTCIndex[iCheck]) {
											model.Indices.push_back(iCheck);        // Set index for this vertex
											vertAlreadyExists = true;       // If we've made it here, the vertex already exists
										}
									}
								}
							}

							if(!vertAlreadyExists) {
								vertPosIndex.push_back(vertPosIndexTemp);
								vertTCIndex.push_back(vertTCIndexTemp);
								vertNormIndex.push_back(vertNormIndexTemp);
								totalVerts++;                       // New vertex created, add to total verts
								model.Indices.push_back(totalVerts - 1);  // Set index for this vertex
							}

							// Set the second vertex for the next triangle to the last vertex we got        
							lastVIndex = model.Indices[vIndex]; // The last vertex index of this TRIANGLE

							meshTriangles++;    // New triangle defined
							vIndex++;
						}
					}
				}
				break;

			case 'm': // mtllib - material library filename
				checkChar = fileIn.get();
				if(checkChar == 't') {
					checkChar = fileIn.get();
					if(checkChar == 'l') {
						checkChar = fileIn.get();
						if(checkChar == 'l') {
							checkChar = fileIn.get();
							if(checkChar == 'i') {
								checkChar = fileIn.get();
								if(checkChar == 'b') {
									checkChar = fileIn.get();
									if(checkChar == ' ') {
										// Store the material libraries file name
										fileIn >> meshMatLib;
									}
								}
							}
						}
					}
				}

				break;

			case 'u': // usemtl - which material to use
				checkChar = fileIn.get();
				if(checkChar == 's') {
					checkChar = fileIn.get();
					if(checkChar == 'e') {
						checkChar = fileIn.get();
						if(checkChar == 'm') {
							checkChar = fileIn.get();
							if(checkChar == 't') {
								checkChar = fileIn.get();
								if(checkChar == 'l') {
									checkChar = fileIn.get();
									if(checkChar == ' ') {
										meshMaterialsTemp = L"";  // Make sure this is cleared

										fileIn >> meshMaterialsTemp; // Get next type (string)

										meshMaterials.push_back(meshMaterialsTemp);
									}
								}
							}
						}
					}
				}
				break;

			default:
				break;
			}
		}
	} else    // If we could not open the file
	{
		SwapChain->SetFullscreenState(false,NULL); // Make sure we are out of fullscreen

													  // create message
		std::wstring message = L"Could not open: ";
		message += Filename;

		MessageBox(0,message.c_str(),  // Display message
			L"Error",MB_OK);

		return false;
	}

	model.SubsetIndexStart.push_back(vIndex); // There won't be another index start after our last subset, so set it here

												// sometimes "g" is defined at the very top of the file, then again before the first group of faces.
												// This makes sure the first subset does not conatain "0" indices.
	if(model.SubsetIndexStart[1] == 0) {
		model.SubsetIndexStart.erase(model.SubsetIndexStart.begin() + 1);
		model.Subsets--;
	}

	// Make sure we have a default for the tex coord and normal
	// if one or both are not specified
	if(!hasNorm)
		vertNorm.push_back(XMFLOAT3(0.0f,0.0f,0.0f));
	if(!hasTexCoord)
		vertTexCoord.push_back(XMFLOAT2(0.0f,0.0f));

	// Close the obj file, and open the mtl file
	fileIn.close();
	setpath = path;
	fileIn.open(setpath.append(meshMatLib).c_str());
	setpath.clear();

	std::wstring lastStringRead;
	int matCount = material.size(); // Total materials

	if(fileIn) {
		while(fileIn) {
			checkChar = fileIn.get();   // Get next char

			switch(checkChar) {
				// Check for comment
			case '#':
				checkChar = fileIn.get();
				while(checkChar != '\n')
					checkChar = fileIn.get();
				break;

				// Set the colors
			case 'K':
				checkChar = fileIn.get();
				if(checkChar == 'd')  // Diffuse Color
				{
					checkChar = fileIn.get();   // Remove space

					fileIn >> material[matCount - 1].Diffuse.x;
					fileIn >> material[matCount - 1].Diffuse.y;
					fileIn >> material[matCount - 1].Diffuse.z;
				}

				if(checkChar == 'a')  // Ambient Color
				{
					checkChar = fileIn.get();   // Remove space

					fileIn >> material[matCount - 1].Ambient.x;
					fileIn >> material[matCount - 1].Ambient.y;
					fileIn >> material[matCount - 1].Ambient.z;
				}

				if(checkChar == 's')  // Ambient Color
				{
					checkChar = fileIn.get();   // Remove space

					fileIn >> material[matCount - 1].Specular.x;
					fileIn >> material[matCount - 1].Specular.y;
					fileIn >> material[matCount - 1].Specular.z;
				}
				break;

			case 'N':
				checkChar = fileIn.get();

				if(checkChar == 's')  // Specular Power (Coefficient)
				{
					checkChar = fileIn.get();   // Remove space

					fileIn >> material[matCount - 1].Specular.w;
				}

				break;

				// Check for transparency
			case 'T':
				checkChar = fileIn.get();
				if(checkChar == 'r') {
					checkChar = fileIn.get();   // Remove space
					float Transparency;
					fileIn >> Transparency;

					material[matCount - 1].Diffuse.w = Transparency;

					if(Transparency > 0.0f)
						material[matCount - 1].IsTransparent = true;
				}
				break;

				// Some obj files specify d for transparency
			case 'd':
				checkChar = fileIn.get();
				if(checkChar == ' ') {
					float Transparency;
					fileIn >> Transparency;

					// 'd' - 0 being most transparent, and 1 being opaque, opposite of Tr
					Transparency = 1.0f - Transparency;

					material[matCount - 1].Diffuse.w = Transparency;

					if(Transparency > 0.0f)
						material[matCount - 1].IsTransparent = true;
				}
				break;

				// Get the diffuse map (texture)
			case 'm':
				checkChar = fileIn.get();
				if(checkChar == 'a') {
					checkChar = fileIn.get();
					if(checkChar == 'p') {
						checkChar = fileIn.get();
						if(checkChar == '_') {
							// map_Kd - Diffuse map
							checkChar = fileIn.get();
							if(checkChar == 'K') {
								checkChar = fileIn.get();
								if(checkChar == 'd') {
									std::wstring fileNamePath;

									fileIn.get();   // Remove whitespace between map_Kd and file

													// Get the file path - We read the pathname char by char since
													// pathnames can sometimes contain spaces, so we will read until
													// we find the file extension
									bool texFilePathEnd = false;
									while(!texFilePathEnd) {
										checkChar = fileIn.get();

										fileNamePath += checkChar;

										if(checkChar == '.') {
											for(int i = 0; i < 3; ++i)
												fileNamePath += fileIn.get();

											texFilePathEnd = true;
										}
									}

									//check if this texture has already been loaded
									bool alreadyLoaded = false;
									for(int i = 0; i < textureMgr.TextureNameArray.size(); ++i) {
										if(fileNamePath == textureMgr.TextureNameArray[i]) {
											alreadyLoaded = true;
											material[matCount - 1].DiffuseTextureID = i;
											material[matCount - 1].HasDiffTexture = true;
										}
									}

									//if the texture is not already loaded, load it now
									if(!alreadyLoaded) {
										ID3D11ShaderResourceView* tempSRV;
										setpath = path;
										hr = CreateTexture(device,setpath.append(fileNamePath).c_str(),&tempSRV);
										setpath.clear();
										if(SUCCEEDED(hr)) {
											textureMgr.TextureNameArray.push_back(fileNamePath.c_str());
											material[matCount - 1].DiffuseTextureID = textureMgr.TextureList.size();
											textureMgr.TextureList.push_back(tempSRV);
											material[matCount - 1].HasDiffTexture = true;
										}
									}
								}

								// Get Ambient Map (texture)
								if(checkChar == 'a') {
									std::wstring fileNamePath;

									fileIn.get();   // Remove whitespace between map_Kd and file

													// Get the file path - We read the pathname char by char since
													// pathnames can sometimes contain spaces, so we will read until
													// we find the file extension
									bool texFilePathEnd = false;
									while(!texFilePathEnd) {
										checkChar = fileIn.get();

										fileNamePath += checkChar;

										if(checkChar == '.') {
											for(int i = 0; i < 3; ++i)
												fileNamePath += fileIn.get();

											texFilePathEnd = true;
										}
									}

									//check if this texture has already been loaded
									bool alreadyLoaded = false;
									for(int i = 0; i < textureMgr.TextureNameArray.size(); ++i) {
										if(fileNamePath == textureMgr.TextureNameArray[i]) {
											alreadyLoaded = true;
											material[matCount - 1].AmbientTextureID = i;
											material[matCount - 1].HasAmbientTexture = true;
										}
									}

									//if the texture is not already loaded, load it now
									if(!alreadyLoaded) {
										ID3D11ShaderResourceView* tempSRV;
										setpath = path;
										hr = CreateTexture(device,setpath.append(fileNamePath).c_str(),&tempSRV);
										setpath.clear();
										if(SUCCEEDED(hr)) {
											textureMgr.TextureNameArray.push_back(fileNamePath.c_str());
											material[matCount - 1].AmbientTextureID = textureMgr.TextureList.size();
											textureMgr.TextureList.push_back(tempSRV);
											material[matCount - 1].HasAmbientTexture = true;
										}
									}
								}

								// Get Specular Map (texture)
								if(checkChar == 's') {
									std::wstring fileNamePath;

									fileIn.get();   // Remove whitespace between map_Ks and file

													// Get the file path - We read the pathname char by char since
													// pathnames can sometimes contain spaces, so we will read until
													// we find the file extension
									bool texFilePathEnd = false;
									while(!texFilePathEnd) {
										checkChar = fileIn.get();

										fileNamePath += checkChar;

										if(checkChar == '.') {
											for(int i = 0; i < 3; ++i)
												fileNamePath += fileIn.get();

											texFilePathEnd = true;
										}
									}

									//check if this texture has already been loaded
									bool alreadyLoaded = false;
									for(int i = 0; i < textureMgr.TextureNameArray.size(); ++i) {
										if(fileNamePath == textureMgr.TextureNameArray[i]) {
											alreadyLoaded = true;
											material[matCount - 1].SpecularTextureID = i;
											material[matCount - 1].HasSpecularTexture = true;
										}
									}

									//if the texture is not already loaded, load it now
									if(!alreadyLoaded) {
										ID3D11ShaderResourceView* tempSRV;
										setpath = path;
										hr = CreateTexture(device,setpath.append(fileNamePath).c_str(),&tempSRV);
										setpath.clear();
										if(SUCCEEDED(hr)) {
											textureMgr.TextureNameArray.push_back(fileNamePath.c_str());
											material[matCount - 1].SpecularTextureID = textureMgr.TextureList.size();
											textureMgr.TextureList.push_back(tempSRV);
											material[matCount - 1].HasSpecularTexture = true;
										}
									}
								}
							}

							//map_d - alpha map
							else if(checkChar == 'd') {
								std::wstring fileNamePath;

								fileIn.get();   // Remove whitespace between map_Ks and file

												// Get the file path - We read the pathname char by char since
												// pathnames can sometimes contain spaces, so we will read until
												// we find the file extension
								bool texFilePathEnd = false;
								while(!texFilePathEnd) {
									checkChar = fileIn.get();

									fileNamePath += checkChar;

									if(checkChar == '.') {
										for(int i = 0; i < 3; ++i)
											fileNamePath += fileIn.get();

										texFilePathEnd = true;
									}
								}

								//check if this texture has already been loaded
								bool alreadyLoaded = false;
								for(int i = 0; i < textureMgr.TextureNameArray.size(); ++i) {
									if(fileNamePath == textureMgr.TextureNameArray[i]) {
										alreadyLoaded = true;
										material[matCount - 1].AlphaTextureID = i;
										material[matCount - 1].IsTransparent = true;
									}
								}

								//if the texture is not already loaded, load it now
								if(!alreadyLoaded) {
									ID3D11ShaderResourceView* tempSRV;
									setpath = path;
									hr = CreateTexture(device,setpath.append(fileNamePath).c_str(),&tempSRV);
									setpath.clear();
									if(SUCCEEDED(hr)) {
										textureMgr.TextureNameArray.push_back(fileNamePath.c_str());
										material[matCount - 1].AlphaTextureID = textureMgr.TextureList.size();
										textureMgr.TextureList.push_back(tempSRV);
										material[matCount - 1].IsTransparent = true;
									}
								}
							}

							// map_bump - bump map (Normal Map)
							else if(checkChar == 'b') {
								checkChar = fileIn.get();
								if(checkChar == 'u') {
									checkChar = fileIn.get();
									if(checkChar == 'm') {
										checkChar = fileIn.get();
										if(checkChar == 'p') {
											std::wstring fileNamePath;

											fileIn.get();   // Remove whitespace between map_bump and file

															// Get the file path - We read the pathname char by char since
															// pathnames can sometimes contain spaces, so we will read until
															// we find the file extension
											bool texFilePathEnd = false;
											while(!texFilePathEnd) {
												checkChar = fileIn.get();

												fileNamePath += checkChar;

												if(checkChar == '.') {
													for(int i = 0; i < 3; ++i)
														fileNamePath += fileIn.get();

													texFilePathEnd = true;
												}
											}

											//check if this texture has already been loaded
											bool alreadyLoaded = false;
											for(int i = 0; i < textureMgr.TextureNameArray.size(); ++i) {
												if(fileNamePath == textureMgr.TextureNameArray[i]) {
													alreadyLoaded = true;
													material[matCount - 1].NormMapTextureID = i;
													material[matCount - 1].HasNormMap = true;
												}
											}

											//if the texture is not already loaded, load it now
											if(!alreadyLoaded) {
												ID3D11ShaderResourceView* tempSRV;
												setpath = path;
												hr = CreateTexture(device,setpath.append(fileNamePath).c_str(),&tempSRV);
												setpath.clear();
												if(SUCCEEDED(hr)) {
													textureMgr.TextureNameArray.push_back(fileNamePath.c_str());
													material[matCount - 1].NormMapTextureID = textureMgr.TextureList.size();
													textureMgr.TextureList.push_back(tempSRV);
													material[matCount - 1].HasNormMap = true;
												}
											}
										}
									}
								}
							}
						}
					}
				}
				break;

			case 'n': // newmtl - Declare new material
				checkChar = fileIn.get();
				if(checkChar == 'e') {
					checkChar = fileIn.get();
					if(checkChar == 'w') {
						checkChar = fileIn.get();
						if(checkChar == 'm') {
							checkChar = fileIn.get();
							if(checkChar == 't') {
								checkChar = fileIn.get();
								if(checkChar == 'l') {
									checkChar = fileIn.get();
									if(checkChar == ' ') {
										// New material, set its defaults
										SurfaceMaterial tempMat;
										material.push_back(tempMat);
										fileIn >> material[matCount].MatName;
										material[matCount].IsTransparent = false;
										material[matCount].HasDiffTexture = false;
										material[matCount].HasAmbientTexture = false;
										material[matCount].HasSpecularTexture = false;
										material[matCount].HasAlphaTexture = false;
										material[matCount].HasNormMap = false;
										material[matCount].NormMapTextureID = 0;
										material[matCount].DiffuseTextureID = 0;
										material[matCount].AlphaTextureID = 0;
										material[matCount].SpecularTextureID = 0;
										material[matCount].AmbientTextureID = 0;
										material[matCount].Specular = XMFLOAT4(0,0,0,0);
										material[matCount].Ambient = XMFLOAT3(0,0,0);
										material[matCount].Diffuse = XMFLOAT4(0,0,0,0);
										matCount++;
									}
								}
							}
						}
					}
				}
			case 'b':
				// map_bump - bump map (Normal Map)
				checkChar = fileIn.get();
				if(checkChar == 'u') {
					checkChar = fileIn.get();
					if(checkChar == 'm') {
						checkChar = fileIn.get();
						if(checkChar == 'p') {
							std::wstring fileNamePath;

							fileIn.get();   // Remove whitespace between map_bump and file

											// Get the file path - We read the pathname char by char since
											// pathnames can sometimes contain spaces, so we will read until
											// we find the file extension
							bool texFilePathEnd = false;
							while(!texFilePathEnd) {
								checkChar = fileIn.get();

								fileNamePath += checkChar;

								if(checkChar == '.') {
									for(int i = 0; i < 3; ++i)
										fileNamePath += fileIn.get();

									texFilePathEnd = true;
								}
							}

							//check if this texture has already been loaded
							bool alreadyLoaded = false;
							for(int i = 0; i < textureMgr.TextureNameArray.size(); ++i) {
								if(fileNamePath == textureMgr.TextureNameArray[i]) {
									alreadyLoaded = true;
									material[matCount - 1].NormMapTextureID = i;
									material[matCount - 1].HasNormMap = true;
								}
							}

							//if the texture is not already loaded, load it now
							if(!alreadyLoaded) {
								ID3D11ShaderResourceView* tempSRV;
								setpath = path;
								hr = CreateTexture(device,setpath.append(fileNamePath).c_str(),&tempSRV);
								setpath.clear();
								if(SUCCEEDED(hr)) {
									textureMgr.TextureNameArray.push_back(fileNamePath.c_str());
									material[matCount - 1].NormMapTextureID = textureMgr.TextureList.size();
									textureMgr.TextureList.push_back(tempSRV);
									material[matCount - 1].HasNormMap = true;
								}
							}
						}
					}
				}
				break;

			default:
				break;
			}
		}
	} else    // If we could not open the material library
	{
		SwapChain->SetFullscreenState(false,NULL); // Make sure we are out of fullscreen

		std::wstring message = L"Could not open: ";
		message += meshMatLib;

		MessageBox(0,message.c_str(),
			L"Error",MB_OK);

		return false;
	}

	// Set the subsets material to the index value
	// of the its material in our material array
	for(int i = 0; i < model.Subsets; ++i) {
		bool hasMat = false;
		for(int j = 0; j < material.size(); ++j) {
			if(meshMaterials[i] == material[j].MatName) {
				model.SubsetMaterialID.push_back(j);
				hasMat = true;
			}
		}
		if(!hasMat)
			model.SubsetMaterialID.push_back(0); // Use first material in array
	}

	std::vector<Vertex> vertices;
	Vertex tempVert;

	// Create our vertices using the information we got 
	// from the file and store them in a vector
	for(int j = 0; j < totalVerts; ++j) {
		tempVert.pos = vertPos[vertPosIndex[j]];
		tempVert.normal = vertNorm[vertNormIndex[j]];
		tempVert.texCoord = vertTexCoord[vertTCIndex[j]];

		vertices.push_back(tempVert);
		model.Vertices.push_back(tempVert.pos);
	}

	//If computeNormals was set to true then we will create our own
	//normals, if it was set to false we will use the obj files normals
	if(ComputeNormals) {
		std::vector<XMFLOAT3> tempNormal;

		//normalized and unnormalized normals
		XMFLOAT3 unnormalized = XMFLOAT3(0.0f,0.0f,0.0f);

		//tangent stuff
		std::vector<XMFLOAT3> tempTangent;
		XMFLOAT3 tangent = XMFLOAT3(0.0f,0.0f,0.0f);
		float tcU1,tcV1,tcU2,tcV2;

		//Used to get vectors (sides) from the position of the verts
		float vecX,vecY,vecZ;

		//Two edges of our triangle
		XMVECTOR edge1 = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
		XMVECTOR edge2 = XMVectorSet(0.0f,0.0f,0.0f,0.0f);

		//Compute face normals
		//And Tangents
		for(int i = 0; i < meshTriangles; ++i) {
			//Get the vector describing one edge of our triangle (edge 0,2)
			vecX = vertices[model.Indices[(i * 3)]].pos.x - vertices[model.Indices[(i * 3) + 2]].pos.x;
			vecY = vertices[model.Indices[(i * 3)]].pos.y - vertices[model.Indices[(i * 3) + 2]].pos.y;
			vecZ = vertices[model.Indices[(i * 3)]].pos.z - vertices[model.Indices[(i * 3) + 2]].pos.z;
			edge1 = XMVectorSet(vecX,vecY,vecZ,0.0f);    //Create our first edge

														 //Get the vector describing another edge of our triangle (edge 2,1)
			vecX = vertices[model.Indices[(i * 3) + 2]].pos.x - vertices[model.Indices[(i * 3) + 1]].pos.x;
			vecY = vertices[model.Indices[(i * 3) + 2]].pos.y - vertices[model.Indices[(i * 3) + 1]].pos.y;
			vecZ = vertices[model.Indices[(i * 3) + 2]].pos.z - vertices[model.Indices[(i * 3) + 1]].pos.z;
			edge2 = XMVectorSet(vecX,vecY,vecZ,0.0f);    //Create our second edge

														 //Cross multiply the two edge vectors to get the un-normalized face normal
			XMStoreFloat3(&unnormalized,XMVector3Cross(edge1,edge2));

			tempNormal.push_back(unnormalized);

			//Find first texture coordinate edge 2d vector
			tcU1 = vertices[model.Indices[(i * 3)]].texCoord.x - vertices[model.Indices[(i * 3) + 2]].texCoord.x;
			tcV1 = vertices[model.Indices[(i * 3)]].texCoord.y - vertices[model.Indices[(i * 3) + 2]].texCoord.y;

			//Find second texture coordinate edge 2d vector
			tcU2 = vertices[model.Indices[(i * 3) + 2]].texCoord.x - vertices[model.Indices[(i * 3) + 1]].texCoord.x;
			tcV2 = vertices[model.Indices[(i * 3) + 2]].texCoord.y - vertices[model.Indices[(i * 3) + 1]].texCoord.y;

			//Find tangent using both tex coord edges and position edges
			tangent.x = (tcV1 * XMVectorGetX(edge1) - tcV2 * XMVectorGetX(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
			tangent.y = (tcV1 * XMVectorGetY(edge1) - tcV2 * XMVectorGetY(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
			tangent.z = (tcV1 * XMVectorGetZ(edge1) - tcV2 * XMVectorGetZ(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));

			tempTangent.push_back(tangent);
		}

		//Compute vertex normals (normal Averaging)
		XMVECTOR normalSum = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
		XMVECTOR tangentSum = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
		int facesUsing = 0;
		float tX,tY,tZ;   //temp axis variables

						  //Go through each vertex
		for(int i = 0; i < totalVerts; ++i) {
			//Check which triangles use this vertex
			for(int j = 0; j < meshTriangles; ++j) {
				if(model.Indices[j * 3] == i ||
					model.Indices[(j * 3) + 1] == i ||
					model.Indices[(j * 3) + 2] == i) {
					tX = XMVectorGetX(normalSum) + tempNormal[j].x;
					tY = XMVectorGetY(normalSum) + tempNormal[j].y;
					tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

					normalSum = XMVectorSet(tX,tY,tZ,0.0f);  //If a face is using the vertex, add the unormalized face normal to the normalSum

															 //We can reuse tX, tY, tZ to sum up tangents
					tX = XMVectorGetX(tangentSum) + tempTangent[j].x;
					tY = XMVectorGetY(tangentSum) + tempTangent[j].y;
					tZ = XMVectorGetZ(tangentSum) + tempTangent[j].z;

					tangentSum = XMVectorSet(tX,tY,tZ,0.0f); //sum up face tangents using this vertex

					facesUsing++;
				}
			}

			//Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
			normalSum = normalSum / facesUsing;
			tangentSum = tangentSum / facesUsing;

			//Normalize the normalSum vector and tangent
			normalSum = XMVector3Normalize(normalSum);
			tangentSum = XMVector3Normalize(tangentSum);

			//Store the normal and tangent in our current vertex
			vertices[i].normal.x = XMVectorGetX(normalSum);
			vertices[i].normal.y = XMVectorGetY(normalSum);
			vertices[i].normal.z = XMVectorGetZ(normalSum);

			vertices[i].tangent.x = XMVectorGetX(tangentSum);
			vertices[i].tangent.y = XMVectorGetY(tangentSum);
			vertices[i].tangent.z = XMVectorGetZ(tangentSum);

			//Clear normalSum, tangentSum and facesUsing for next vertex
			normalSum = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
			tangentSum = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
			facesUsing = 0;

		}
	}

	// Create Axis-Aligned Bounding Box (AABB)
	XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX,FLT_MAX,FLT_MAX);
	XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX,-FLT_MAX,-FLT_MAX);
	model.BoundingSphere = 0;

	for(UINT i = 0; i < model.Vertices.size(); i++) {
		// The minVertex and maxVertex will most likely not be actual vertices in the m_model, but vertices
		// that use the smallest and largest x, y, and z values from the m_model to be sure ALL vertices are
		// covered by the bounding volume

		//Get the smallest vertex 
		minVertex.x = min(minVertex.x,model.Vertices[i].x);	// Find smallest x value in m_model
		minVertex.y = min(minVertex.y,model.Vertices[i].y);	// Find smallest y value in m_model
		minVertex.z = min(minVertex.z,model.Vertices[i].z);	// Find smallest z value in m_model

																//Get the largest vertex 
		maxVertex.x = max(maxVertex.x,model.Vertices[i].x);	// Find largest x value in m_model
		maxVertex.y = max(maxVertex.y,model.Vertices[i].y);	// Find largest y value in m_model
		maxVertex.z = max(maxVertex.z,model.Vertices[i].z);	// Find largest z value in m_model
	}

	// Our AABB [0] is the min vertex and [1] is the max
	model.AABB.push_back(minVertex);
	model.AABB.push_back(maxVertex);

	// Get m_models true center
	model.Center.x = maxVertex.x + minVertex.x / 2.0f;
	model.Center.y = maxVertex.y + minVertex.y / 2.0f;
	model.Center.z = maxVertex.z + minVertex.z / 2.0f;

	// Now that we have the center, get the bounding sphere	
	for(UINT i = 0; i < model.Vertices.size(); i++) {
		float x = (model.Center.x - model.Vertices[i].x) * (model.Center.x - model.Vertices[i].x);
		float y = (model.Center.y - model.Vertices[i].y) * (model.Center.y - model.Vertices[i].y);
		float z = (model.Center.z - model.Vertices[i].z) * (model.Center.z - model.Vertices[i].z);

		// Get m_models bounding sphere
		model.BoundingSphere = max(model.BoundingSphere,(x + y + z));
	}

	// We didn't use the square root when finding the largest distance since it slows things down.
	// We can square root the answer from above to get the actual bounding sphere now
	model.BoundingSphere = sqrt(model.BoundingSphere);


	//Create index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc,sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * meshTriangles * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &model.Indices[0];
	device->CreateBuffer(&indexBufferDesc,&iinitData,&model.IndexBuff);

	//Create Vertex Buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc,sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * totalVerts;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData,sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &vertices[0];
	hr = device->CreateBuffer(&vertexBufferDesc,&vertexBufferData,&model.VertBuff);

	return true;
}


bool Model::LoadObjModel(
	ID3D11Device* device,		// Pointer to the device
	std::wstring path,			// object path
	std::wstring Filename,      // obj m_model filename (m_model.obj)
	IDXGISwapChain* SwapChain,	// Pointer to the SwapChain
	bool IsRHCoordSys,          // True if m_model was created in right hand coord system
	bool ComputeNormals,		// True to compute the normals, false to use the files normals
	std::vector<DirectX::XMFLOAT3> AABB
) {
	HRESULT hr = 0;

	std::wstring setpath = path;
	std::wifstream fileIn(setpath.append(Filename).c_str());   // Open file
	setpath.clear();

	std::wstring meshMatLib; // String to hold our obj material library filename (m_model.mtl)

							 // Arrays to store our m_model's information
	std::vector<DWORD> indices;
	std::vector<XMFLOAT3> vertPos;
	std::vector<XMFLOAT3> vertNorm;
	std::vector<XMFLOAT2> vertTexCoord;
	std::vector<std::wstring> meshMaterials;

	// Vertex definition indices
	std::vector<int> vertPosIndex;
	std::vector<int> vertNormIndex;
	std::vector<int> vertTCIndex;

	// Make sure we have a default if no tex coords or normals are defined
	bool hasTexCoord = false;
	bool hasNorm = false;

	// Temp variables to store into vectors
	std::wstring meshMaterialsTemp;
	int vertPosIndexTemp;
	int vertNormIndexTemp;
	int vertTCIndexTemp;

	wchar_t checkChar;      // The variable we will use to store one char from file at a time
	std::wstring face;      // Holds the string containing our face vertices
	int vIndex = 0;         // Keep track of our vertex index count
	int triangleCount = 0;  // Total Triangles
	int totalVerts = 0;
	int meshTriangles = 0;

	model.Center = DirectX::XMFLOAT3(0,0,0);

	//Check to see if the file was opened
	if(fileIn) {
		while(fileIn) {
			checkChar = fileIn.get();   //Get next char

			switch(checkChar) {
				// A comment. Skip rest of the line
			case '#':
				checkChar = fileIn.get();
				while(checkChar != '\n')
					checkChar = fileIn.get();
				break;

				// Get Vertex Descriptions;
			case 'v':
				checkChar = fileIn.get();
				if(checkChar == ' ')  // v - vert position
				{
					float vz,vy,vx;
					fileIn >> vx >> vy >> vz;   // Store the next three types

					if(IsRHCoordSys)            // If m_model is from an RH Coord System
						vertPos.push_back(XMFLOAT3(vx,vy,vz * -1.0f));   // Invert the Z axis
					else
						vertPos.push_back(XMFLOAT3(vx,vy,vz));
				}
				if(checkChar == 't')  // vt - vert tex coords
				{
					float vtcu,vtcv;
					fileIn >> vtcu >> vtcv;     // Store next two types

					if(IsRHCoordSys)            // If m_model is from an RH Coord System
						vertTexCoord.push_back(XMFLOAT2(vtcu,1.0f - vtcv));  // Reverse the "v" axis
					else
						vertTexCoord.push_back(XMFLOAT2(vtcu,vtcv));

					hasTexCoord = true;         // We know the m_model uses texture coords
				}
				if(checkChar == 'n')          // vn - vert normal
				{
					float vnx,vny,vnz;
					fileIn >> vnx >> vny >> vnz;    // Store next three types

					if(IsRHCoordSys)                // If m_model is from an RH Coord System
						vertNorm.push_back(XMFLOAT3(vnx,vny,vnz * -1.0f));  // Invert the Z axis
					else
						vertNorm.push_back(XMFLOAT3(vnx,vny,vnz));

					hasNorm = true;                 // We know the m_model defines normals
				}
				break;

				// New group (Subset)
			case 'o': // g - defines a group
				checkChar = fileIn.get();
				if(checkChar == ' ') {
					model.SubsetIndexStart.push_back(vIndex);       // Start index for this subset
					model.Subsets++;
				}
				break;

				// Get Face Index
			case 'f': // f - defines the faces
				checkChar = fileIn.get();
				if(checkChar == ' ') {
					face = L"";               // Holds all vertex definitions for the face (eg. "1\1\1 2\2\2 3\3\3")
					std::wstring VertDef;   // Holds one vertex definition at a time (eg. "1\1\1")
					triangleCount = 0;      // Keep track of the triangles for this face, since we will have to 
											// "retriangulate" faces with more than 3 sides

					checkChar = fileIn.get();
					while(checkChar != '\n') {
						face += checkChar;          // Add the char to our face string
						checkChar = fileIn.get();   // Get the next Character
						if(checkChar == ' ')      // If its a space...
							triangleCount++;        // Increase our triangle count
					}

					// Check for space at the end of our face string
					if(face[face.length() - 1] == ' ')
						triangleCount--;    // Each space adds to our triangle count

					triangleCount -= 1;     // Ever vertex in the face AFTER the first two are new faces

					std::wstringstream ss(face);

					if(face.length() > 0) {
						int firstVIndex,lastVIndex;    // Holds the first and last vertice's index

						for(int i = 0; i < 3; ++i)      // First three vertices (first triangle)
						{
							ss >> VertDef;  // Get vertex definition (vPos/vTexCoord/vNorm)

							std::wstring vertPart;
							int whichPart = 0;      // (vPos, vTexCoord, or vNorm)

													// Parse this string
							for(int j = 0; j < VertDef.length(); ++j) {
								if(VertDef[j] != '/') // If there is no divider "/", add a char to our vertPart
									vertPart += VertDef[j];

								// If the current char is a divider "/", or its the last character in the string
								if(VertDef[j] == '/' || j == VertDef.length() - 1) {
									std::wistringstream wstringToInt(vertPart); // Used to convert wstring to int

									if(whichPart == 0)  // If it's the vPos
									{
										wstringToInt >> vertPosIndexTemp;
										vertPosIndexTemp -= 1;      // subtract one since c++ arrays start with 0, and obj start with 1

																	// Check to see if the vert pos was the only thing specified
										if(j == VertDef.length() - 1) {
											vertNormIndexTemp = 0;
											vertTCIndexTemp = 0;
										}
									}

									else if(whichPart == 1) // If vTexCoord
									{
										if(vertPart != L"")   // Check to see if there even is a tex coord
										{
											wstringToInt >> vertTCIndexTemp;
											vertTCIndexTemp -= 1;   // subtract one since c++ arrays start with 0, and obj start with 1
										} else    // If there is no tex coord, make a default
											vertTCIndexTemp = 0;

										// If the cur. char is the second to last in the string, then
										// there must be no normal, so set a default normal
										if(j == VertDef.length() - 1)
											vertNormIndexTemp = 0;

									} else if(whichPart == 2) // If vNorm
									{
										std::wistringstream wstringToInt(vertPart);

										wstringToInt >> vertNormIndexTemp;
										vertNormIndexTemp -= 1;     // subtract one since c++ arrays start with 0, and obj start with 1
									}

									vertPart = L"";   // Get ready for next vertex part
									whichPart++;    // Move on to next vertex part                  
								}
							}

							// Check to make sure there is at least one subset
							if(model.Subsets == 0) {
								model.SubsetIndexStart.push_back(vIndex);       // Start index for this subset
								model.Subsets++;
							}

							// Avoid duplicate vertices
							bool vertAlreadyExists = false;
							if(totalVerts >= 3) // Make sure we at least have one triangle to check
							{
								// Loop through all the vertices
								for(int iCheck = 0; iCheck < totalVerts; ++iCheck) {
									// If the vertex position and texture coordinate in memory are the same
									// As the vertex position and texture coordinate we just now got out
									// of the obj file, we will set this faces vertex index to the vertex's
									// index value in memory. This makes sure we don't create duplicate vertices
									if(vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists) {
										if(vertTCIndexTemp == vertTCIndex[iCheck]) {
											model.Indices.push_back(iCheck);        // Set index for this vertex
											vertAlreadyExists = true;       // If we've made it here, the vertex already exists
										}
									}
								}
							}

							// If this vertex is not already in our vertex arrays, put it there
							if(!vertAlreadyExists) {
								vertPosIndex.push_back(vertPosIndexTemp);
								vertTCIndex.push_back(vertTCIndexTemp);
								vertNormIndex.push_back(vertNormIndexTemp);
								totalVerts++;   // We created a new vertex
								model.Indices.push_back(totalVerts - 1);  // Set index for this vertex
							}

							// If this is the very first vertex in the face, we need to
							// make sure the rest of the triangles use this vertex
							if(i == 0) {
								firstVIndex = model.Indices[vIndex];    //The first vertex index of this FACE

							}

							// If this was the last vertex in the first triangle, we will make sure
							// the next triangle uses this one (eg. tri1(1,2,3) tri2(1,3,4) tri3(1,4,5))
							if(i == 2) {
								lastVIndex = model.Indices[vIndex]; // The last vertex index of this TRIANGLE
							}
							vIndex++;   // Increment index count
						}

						meshTriangles++;    // One triangle down

											// If there are more than three vertices in the face definition, we need to make sure
											// we convert the face to triangles. We created our first triangle above, now we will
											// create a new triangle for every new vertex in the face, using the very first vertex
											// of the face, and the last vertex from the triangle before the current triangle
						for(int l = 0; l < triangleCount - 1; ++l)    // Loop through the next vertices to create new triangles
						{
							// First vertex of this triangle (the very first vertex of the face too)
							model.Indices.push_back(firstVIndex);           // Set index for this vertex
							vIndex++;

							// Second Vertex of this triangle (the last vertex used in the tri before this one)
							model.Indices.push_back(lastVIndex);            // Set index for this vertex
							vIndex++;

							// Get the third vertex for this triangle
							ss >> VertDef;

							std::wstring vertPart;
							int whichPart = 0;

							// Parse this string (same as above)
							for(int j = 0; j < VertDef.length(); ++j) {
								if(VertDef[j] != '/')
									vertPart += VertDef[j];
								if(VertDef[j] == '/' || j == VertDef.length() - 1) {
									std::wistringstream wstringToInt(vertPart);

									if(whichPart == 0) {
										wstringToInt >> vertPosIndexTemp;
										vertPosIndexTemp -= 1;

										// Check to see if the vert pos was the only thing specified
										if(j == VertDef.length() - 1) {
											vertTCIndexTemp = 0;
											vertNormIndexTemp = 0;
										}
									} else if(whichPart == 1) {
										if(vertPart != L"") {
											wstringToInt >> vertTCIndexTemp;
											vertTCIndexTemp -= 1;
										} else
											vertTCIndexTemp = 0;
										if(j == VertDef.length() - 1)
											vertNormIndexTemp = 0;

									} else if(whichPart == 2) {
										std::wistringstream wstringToInt(vertPart);

										wstringToInt >> vertNormIndexTemp;
										vertNormIndexTemp -= 1;
									}

									vertPart = L"";
									whichPart++;
								}
							}

							// Check for duplicate vertices
							bool vertAlreadyExists = false;
							if(totalVerts >= 3) // Make sure we at least have one triangle to check
							{
								for(int iCheck = 0; iCheck < totalVerts; ++iCheck) {
									if(vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists) {
										if(vertTCIndexTemp == vertTCIndex[iCheck]) {
											model.Indices.push_back(iCheck);        // Set index for this vertex
											vertAlreadyExists = true;       // If we've made it here, the vertex already exists
										}
									}
								}
							}

							if(!vertAlreadyExists) {
								vertPosIndex.push_back(vertPosIndexTemp);
								vertTCIndex.push_back(vertTCIndexTemp);
								vertNormIndex.push_back(vertNormIndexTemp);
								totalVerts++;                       // New vertex created, add to total verts
								model.Indices.push_back(totalVerts - 1);  // Set index for this vertex
							}

							// Set the second vertex for the next triangle to the last vertex we got        
							lastVIndex = model.Indices[vIndex]; // The last vertex index of this TRIANGLE

							meshTriangles++;    // New triangle defined
							vIndex++;
						}
					}
				}
				break;

			case 'm': // mtllib - material library filename
				checkChar = fileIn.get();
				if(checkChar == 't') {
					checkChar = fileIn.get();
					if(checkChar == 'l') {
						checkChar = fileIn.get();
						if(checkChar == 'l') {
							checkChar = fileIn.get();
							if(checkChar == 'i') {
								checkChar = fileIn.get();
								if(checkChar == 'b') {
									checkChar = fileIn.get();
									if(checkChar == ' ') {
										// Store the material libraries file name
										fileIn >> meshMatLib;
									}
								}
							}
						}
					}
				}

				break;

			case 'u': // usemtl - which material to use
				checkChar = fileIn.get();
				if(checkChar == 's') {
					checkChar = fileIn.get();
					if(checkChar == 'e') {
						checkChar = fileIn.get();
						if(checkChar == 'm') {
							checkChar = fileIn.get();
							if(checkChar == 't') {
								checkChar = fileIn.get();
								if(checkChar == 'l') {
									checkChar = fileIn.get();
									if(checkChar == ' ') {
										meshMaterialsTemp = L"";  // Make sure this is cleared

										fileIn >> meshMaterialsTemp; // Get next type (string)

										meshMaterials.push_back(meshMaterialsTemp);
									}
								}
							}
						}
					}
				}
				break;

			default:
				break;
			}
		}
	} else    // If we could not open the file
	{
		SwapChain->SetFullscreenState(false,NULL); // Make sure we are out of fullscreen

												   // create message
		std::wstring message = L"Could not open: ";
		message += Filename;

		MessageBox(0,message.c_str(),  // Display message
			L"Error",MB_OK);

		return false;
	}

	model.SubsetIndexStart.push_back(vIndex); // There won't be another index start after our last subset, so set it here

											  // sometimes "g" is defined at the very top of the file, then again before the first group of faces.
											  // This makes sure the first subset does not conatain "0" indices.
	if(model.SubsetIndexStart[1] == 0) {
		model.SubsetIndexStart.erase(model.SubsetIndexStart.begin() + 1);
		model.Subsets--;
	}

	// Make sure we have a default for the tex coord and normal
	// if one or both are not specified
	if(!hasNorm)
		vertNorm.push_back(XMFLOAT3(0.0f,0.0f,0.0f));
	if(!hasTexCoord)
		vertTexCoord.push_back(XMFLOAT2(0.0f,0.0f));

	// Close the obj file, and open the mtl file
	fileIn.close();
	setpath = path;
	fileIn.open(setpath.append(meshMatLib).c_str());
	setpath.clear();

	std::wstring lastStringRead;
	int matCount = material.size(); // Total materials

	if(fileIn) {
		while(fileIn) {
			checkChar = fileIn.get();   // Get next char

			switch(checkChar) {
				// Check for comment
			case '#':
				checkChar = fileIn.get();
				while(checkChar != '\n')
					checkChar = fileIn.get();
				break;

				// Set the colors
			case 'K':
				checkChar = fileIn.get();
				if(checkChar == 'd')  // Diffuse Color
				{
					checkChar = fileIn.get();   // Remove space

					fileIn >> material[matCount - 1].Diffuse.x;
					fileIn >> material[matCount - 1].Diffuse.y;
					fileIn >> material[matCount - 1].Diffuse.z;
				}

				if(checkChar == 'a')  // Ambient Color
				{
					checkChar = fileIn.get();   // Remove space

					fileIn >> material[matCount - 1].Ambient.x;
					fileIn >> material[matCount - 1].Ambient.y;
					fileIn >> material[matCount - 1].Ambient.z;
				}

				if(checkChar == 's')  // Ambient Color
				{
					checkChar = fileIn.get();   // Remove space

					fileIn >> material[matCount - 1].Specular.x;
					fileIn >> material[matCount - 1].Specular.y;
					fileIn >> material[matCount - 1].Specular.z;
				}
				break;

			case 'N':
				checkChar = fileIn.get();

				if(checkChar == 's')  // Specular Power (Coefficient)
				{
					checkChar = fileIn.get();   // Remove space

					fileIn >> material[matCount - 1].Specular.w;
				}

				break;

				// Check for transparency
			case 'T':
				checkChar = fileIn.get();
				if(checkChar == 'r') {
					checkChar = fileIn.get();   // Remove space
					float Transparency;
					fileIn >> Transparency;

					material[matCount - 1].Diffuse.w = Transparency;

					if(Transparency > 0.0f)
						material[matCount - 1].IsTransparent = true;
				}
				break;

				// Some obj files specify d for transparency
			case 'd':
				checkChar = fileIn.get();
				if(checkChar == ' ') {
					float Transparency;
					fileIn >> Transparency;

					// 'd' - 0 being most transparent, and 1 being opaque, opposite of Tr
					Transparency = 1.0f - Transparency;

					material[matCount - 1].Diffuse.w = Transparency;

					if(Transparency > 0.0f)
						material[matCount - 1].IsTransparent = true;
				}
				break;

				// Get the diffuse map (texture)
			case 'm':
				checkChar = fileIn.get();
				if(checkChar == 'a') {
					checkChar = fileIn.get();
					if(checkChar == 'p') {
						checkChar = fileIn.get();
						if(checkChar == '_') {
							// map_Kd - Diffuse map
							checkChar = fileIn.get();
							if(checkChar == 'K') {
								checkChar = fileIn.get();
								if(checkChar == 'd') {
									std::wstring fileNamePath;

									fileIn.get();   // Remove whitespace between map_Kd and file

													// Get the file path - We read the pathname char by char since
													// pathnames can sometimes contain spaces, so we will read until
													// we find the file extension
									bool texFilePathEnd = false;
									while(!texFilePathEnd) {
										checkChar = fileIn.get();

										fileNamePath += checkChar;

										if(checkChar == '.') {
											for(int i = 0; i < 3; ++i)
												fileNamePath += fileIn.get();

											texFilePathEnd = true;
										}
									}

									//check if this texture has already been loaded
									bool alreadyLoaded = false;
									for(int i = 0; i < textureMgr.TextureNameArray.size(); ++i) {
										if(fileNamePath == textureMgr.TextureNameArray[i]) {
											alreadyLoaded = true;
											material[matCount - 1].DiffuseTextureID = i;
											material[matCount - 1].HasDiffTexture = true;
										}
									}

									//if the texture is not already loaded, load it now
									if(!alreadyLoaded) {
										ID3D11ShaderResourceView* tempSRV;
										setpath = path;
										hr = CreateTexture(device,setpath.append(fileNamePath).c_str(),&tempSRV);
										setpath.clear();
										if(SUCCEEDED(hr)) {
											textureMgr.TextureNameArray.push_back(fileNamePath.c_str());
											material[matCount - 1].DiffuseTextureID = textureMgr.TextureList.size();
											textureMgr.TextureList.push_back(tempSRV);
											material[matCount - 1].HasDiffTexture = true;
										}
									}
								}

								// Get Ambient Map (texture)
								if(checkChar == 'a') {
									std::wstring fileNamePath;

									fileIn.get();   // Remove whitespace between map_Kd and file

													// Get the file path - We read the pathname char by char since
													// pathnames can sometimes contain spaces, so we will read until
													// we find the file extension
									bool texFilePathEnd = false;
									while(!texFilePathEnd) {
										checkChar = fileIn.get();

										fileNamePath += checkChar;

										if(checkChar == '.') {
											for(int i = 0; i < 3; ++i)
												fileNamePath += fileIn.get();

											texFilePathEnd = true;
										}
									}

									//check if this texture has already been loaded
									bool alreadyLoaded = false;
									for(int i = 0; i < textureMgr.TextureNameArray.size(); ++i) {
										if(fileNamePath == textureMgr.TextureNameArray[i]) {
											alreadyLoaded = true;
											material[matCount - 1].AmbientTextureID = i;
											material[matCount - 1].HasAmbientTexture = true;
										}
									}

									//if the texture is not already loaded, load it now
									if(!alreadyLoaded) {
										ID3D11ShaderResourceView* tempSRV;
										setpath = path;
										hr = CreateTexture(device,setpath.append(fileNamePath).c_str(),&tempSRV);
										setpath.clear();
										if(SUCCEEDED(hr)) {
											textureMgr.TextureNameArray.push_back(fileNamePath.c_str());
											material[matCount - 1].AmbientTextureID = textureMgr.TextureList.size();
											textureMgr.TextureList.push_back(tempSRV);
											material[matCount - 1].HasAmbientTexture = true;
										}
									}
								}

								// Get Specular Map (texture)
								if(checkChar == 's') {
									std::wstring fileNamePath;

									fileIn.get();   // Remove whitespace between map_Ks and file

													// Get the file path - We read the pathname char by char since
													// pathnames can sometimes contain spaces, so we will read until
													// we find the file extension
									bool texFilePathEnd = false;
									while(!texFilePathEnd) {
										checkChar = fileIn.get();

										fileNamePath += checkChar;

										if(checkChar == '.') {
											for(int i = 0; i < 3; ++i)
												fileNamePath += fileIn.get();

											texFilePathEnd = true;
										}
									}

									//check if this texture has already been loaded
									bool alreadyLoaded = false;
									for(int i = 0; i < textureMgr.TextureNameArray.size(); ++i) {
										if(fileNamePath == textureMgr.TextureNameArray[i]) {
											alreadyLoaded = true;
											material[matCount - 1].SpecularTextureID = i;
											material[matCount - 1].HasSpecularTexture = true;
										}
									}

									//if the texture is not already loaded, load it now
									if(!alreadyLoaded) {
										ID3D11ShaderResourceView* tempSRV;
										setpath = path;
										hr = CreateTexture(device,setpath.append(fileNamePath).c_str(),&tempSRV);
										setpath.clear();
										if(SUCCEEDED(hr)) {
											textureMgr.TextureNameArray.push_back(fileNamePath.c_str());
											material[matCount - 1].SpecularTextureID = textureMgr.TextureList.size();
											textureMgr.TextureList.push_back(tempSRV);
											material[matCount - 1].HasSpecularTexture = true;
										}
									}
								}
							}

							//map_d - alpha map
							else if(checkChar == 'd') {
								std::wstring fileNamePath;

								fileIn.get();   // Remove whitespace between map_Ks and file

												// Get the file path - We read the pathname char by char since
												// pathnames can sometimes contain spaces, so we will read until
												// we find the file extension
								bool texFilePathEnd = false;
								while(!texFilePathEnd) {
									checkChar = fileIn.get();

									fileNamePath += checkChar;

									if(checkChar == '.') {
										for(int i = 0; i < 3; ++i)
											fileNamePath += fileIn.get();

										texFilePathEnd = true;
									}
								}

								//check if this texture has already been loaded
								bool alreadyLoaded = false;
								for(int i = 0; i < textureMgr.TextureNameArray.size(); ++i) {
									if(fileNamePath == textureMgr.TextureNameArray[i]) {
										alreadyLoaded = true;
										material[matCount - 1].AlphaTextureID = i;
										material[matCount - 1].IsTransparent = true;
									}
								}

								//if the texture is not already loaded, load it now
								if(!alreadyLoaded) {
									ID3D11ShaderResourceView* tempSRV;
									setpath = path;
									hr = CreateTexture(device,setpath.append(fileNamePath).c_str(),&tempSRV);
									setpath.clear();
									if(SUCCEEDED(hr)) {
										textureMgr.TextureNameArray.push_back(fileNamePath.c_str());
										material[matCount - 1].AlphaTextureID = textureMgr.TextureList.size();
										textureMgr.TextureList.push_back(tempSRV);
										material[matCount - 1].IsTransparent = true;
									}
								}
							}

							// map_bump - bump map (Normal Map)
							else if(checkChar == 'b') {
								checkChar = fileIn.get();
								if(checkChar == 'u') {
									checkChar = fileIn.get();
									if(checkChar == 'm') {
										checkChar = fileIn.get();
										if(checkChar == 'p') {
											std::wstring fileNamePath;

											fileIn.get();   // Remove whitespace between map_bump and file

															// Get the file path - We read the pathname char by char since
															// pathnames can sometimes contain spaces, so we will read until
															// we find the file extension
											bool texFilePathEnd = false;
											while(!texFilePathEnd) {
												checkChar = fileIn.get();

												fileNamePath += checkChar;

												if(checkChar == '.') {
													for(int i = 0; i < 3; ++i)
														fileNamePath += fileIn.get();

													texFilePathEnd = true;
												}
											}

											//check if this texture has already been loaded
											bool alreadyLoaded = false;
											for(int i = 0; i < textureMgr.TextureNameArray.size(); ++i) {
												if(fileNamePath == textureMgr.TextureNameArray[i]) {
													alreadyLoaded = true;
													material[matCount - 1].NormMapTextureID = i;
													material[matCount - 1].HasNormMap = true;
												}
											}

											//if the texture is not already loaded, load it now
											if(!alreadyLoaded) {
												ID3D11ShaderResourceView* tempSRV;
												setpath = path;
												hr = CreateTexture(device,setpath.append(fileNamePath).c_str(),&tempSRV);
												setpath.clear();
												if(SUCCEEDED(hr)) {
													textureMgr.TextureNameArray.push_back(fileNamePath.c_str());
													material[matCount - 1].NormMapTextureID = textureMgr.TextureList.size();
													textureMgr.TextureList.push_back(tempSRV);
													material[matCount - 1].HasNormMap = true;
												}
											}
										}
									}
								}
							}
						}
					}
				}
				break;

			case 'n': // newmtl - Declare new material
				checkChar = fileIn.get();
				if(checkChar == 'e') {
					checkChar = fileIn.get();
					if(checkChar == 'w') {
						checkChar = fileIn.get();
						if(checkChar == 'm') {
							checkChar = fileIn.get();
							if(checkChar == 't') {
								checkChar = fileIn.get();
								if(checkChar == 'l') {
									checkChar = fileIn.get();
									if(checkChar == ' ') {
										// New material, set its defaults
										SurfaceMaterial tempMat;
										material.push_back(tempMat);
										fileIn >> material[matCount].MatName;
										material[matCount].IsTransparent = false;
										material[matCount].HasDiffTexture = false;
										material[matCount].HasAmbientTexture = false;
										material[matCount].HasSpecularTexture = false;
										material[matCount].HasAlphaTexture = false;
										material[matCount].HasNormMap = false;
										material[matCount].NormMapTextureID = 0;
										material[matCount].DiffuseTextureID = 0;
										material[matCount].AlphaTextureID = 0;
										material[matCount].SpecularTextureID = 0;
										material[matCount].AmbientTextureID = 0;
										material[matCount].Specular = XMFLOAT4(0,0,0,0);
										material[matCount].Ambient = XMFLOAT3(0,0,0);
										material[matCount].Diffuse = XMFLOAT4(0,0,0,0);
										matCount++;
									}
								}
							}
						}
					}
				}
			case 'b':
				// map_bump - bump map (Normal Map)
				checkChar = fileIn.get();
				if(checkChar == 'u') {
					checkChar = fileIn.get();
					if(checkChar == 'm') {
						checkChar = fileIn.get();
						if(checkChar == 'p') {
							std::wstring fileNamePath;

							fileIn.get();   // Remove whitespace between map_bump and file

											// Get the file path - We read the pathname char by char since
											// pathnames can sometimes contain spaces, so we will read until
											// we find the file extension
							bool texFilePathEnd = false;
							while(!texFilePathEnd) {
								checkChar = fileIn.get();

								fileNamePath += checkChar;

								if(checkChar == '.') {
									for(int i = 0; i < 3; ++i)
										fileNamePath += fileIn.get();

									texFilePathEnd = true;
								}
							}

							//check if this texture has already been loaded
							bool alreadyLoaded = false;
							for(int i = 0; i < textureMgr.TextureNameArray.size(); ++i) {
								if(fileNamePath == textureMgr.TextureNameArray[i]) {
									alreadyLoaded = true;
									material[matCount - 1].NormMapTextureID = i;
									material[matCount - 1].HasNormMap = true;
								}
							}

							//if the texture is not already loaded, load it now
							if(!alreadyLoaded) {
								ID3D11ShaderResourceView* tempSRV;
								setpath = path;
								hr = CreateTexture(device,setpath.append(fileNamePath).c_str(),&tempSRV);
								setpath.clear();
								if(SUCCEEDED(hr)) {
									textureMgr.TextureNameArray.push_back(fileNamePath.c_str());
									material[matCount - 1].NormMapTextureID = textureMgr.TextureList.size();
									textureMgr.TextureList.push_back(tempSRV);
									material[matCount - 1].HasNormMap = true;
								}
							}
						}
					}
				}
				break;

			default:
				break;
			}
		}
	} else    // If we could not open the material library
	{
		SwapChain->SetFullscreenState(false,NULL); // Make sure we are out of fullscreen

		std::wstring message = L"Could not open: ";
		message += meshMatLib;

		MessageBox(0,message.c_str(),
			L"Error",MB_OK);

		return false;
	}

	// Set the subsets material to the index value
	// of the its material in our material array
	for(int i = 0; i < model.Subsets; ++i) {
		bool hasMat = false;
		for(int j = 0; j < material.size(); ++j) {
			if(meshMaterials[i] == material[j].MatName) {
				model.SubsetMaterialID.push_back(j);
				hasMat = true;
			}
		}
		if(!hasMat)
			model.SubsetMaterialID.push_back(0); // Use first material in array
	}

	std::vector<Vertex> vertices;
	Vertex tempVert;
	
	// re adjust vertex for AABB
	vertPos[1 - 1].z = AABB[1].z;
	vertPos[2 - 1].z = AABB[1].z;
	vertPos[3 - 1].z = AABB[1].z;
	vertPos[4 - 1].z = AABB[1].z;

	vertPos[5 - 1].z = AABB[0].z;
	vertPos[6 - 1].z = AABB[0].z;
	vertPos[7 - 1].z = AABB[0].z;
	vertPos[8 - 1].z = AABB[0].z;

	vertPos[3 - 1].y = AABB[1].z;
	vertPos[4 - 1].y = AABB[1].z;
	vertPos[5 - 1].y = AABB[1].z;
	vertPos[6 - 1].y = AABB[1].z;

	vertPos[1 - 1].y = AABB[0].y;
	vertPos[2 - 1].y = AABB[0].y;
	vertPos[7 - 1].y = AABB[0].y;
	vertPos[8 - 1].y = AABB[0].y;

	vertPos[2 - 1].x = AABB[1].x;
	vertPos[4 - 1].x = AABB[1].x;
	vertPos[6 - 1].x = AABB[1].x;
	vertPos[8 - 1].x = AABB[1].x;

	vertPos[1 - 1].x = AABB[0].x;
	vertPos[3 - 1].x = AABB[0].x;
	vertPos[5 - 1].x = AABB[0].x;
	vertPos[7 - 1].x = AABB[0].x;


	// Create our vertices using the information we got 
	// from the file and store them in a vector
	for(int j = 0; j < totalVerts; ++j) {
		tempVert.pos = vertPos[vertPosIndex[j]];
		tempVert.normal = vertNorm[vertNormIndex[j]];
		tempVert.texCoord = vertTexCoord[vertTCIndex[j]];

		vertices.push_back(tempVert);
		model.Vertices.push_back(tempVert.pos);
	}

	//If computeNormals was set to true then we will create our own
	//normals, if it was set to false we will use the obj files normals
	if(ComputeNormals) {
		std::vector<XMFLOAT3> tempNormal;

		//normalized and unnormalized normals
		XMFLOAT3 unnormalized = XMFLOAT3(0.0f,0.0f,0.0f);

		//tangent stuff
		std::vector<XMFLOAT3> tempTangent;
		XMFLOAT3 tangent = XMFLOAT3(0.0f,0.0f,0.0f);
		float tcU1,tcV1,tcU2,tcV2;

		//Used to get vectors (sides) from the position of the verts
		float vecX,vecY,vecZ;

		//Two edges of our triangle
		XMVECTOR edge1 = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
		XMVECTOR edge2 = XMVectorSet(0.0f,0.0f,0.0f,0.0f);

		//Compute face normals
		//And Tangents
		for(int i = 0; i < meshTriangles; ++i) {
			//Get the vector describing one edge of our triangle (edge 0,2)
			vecX = vertices[model.Indices[(i * 3)]].pos.x - vertices[model.Indices[(i * 3) + 2]].pos.x;
			vecY = vertices[model.Indices[(i * 3)]].pos.y - vertices[model.Indices[(i * 3) + 2]].pos.y;
			vecZ = vertices[model.Indices[(i * 3)]].pos.z - vertices[model.Indices[(i * 3) + 2]].pos.z;
			edge1 = XMVectorSet(vecX,vecY,vecZ,0.0f);    //Create our first edge

														 //Get the vector describing another edge of our triangle (edge 2,1)
			vecX = vertices[model.Indices[(i * 3) + 2]].pos.x - vertices[model.Indices[(i * 3) + 1]].pos.x;
			vecY = vertices[model.Indices[(i * 3) + 2]].pos.y - vertices[model.Indices[(i * 3) + 1]].pos.y;
			vecZ = vertices[model.Indices[(i * 3) + 2]].pos.z - vertices[model.Indices[(i * 3) + 1]].pos.z;
			edge2 = XMVectorSet(vecX,vecY,vecZ,0.0f);    //Create our second edge

														 //Cross multiply the two edge vectors to get the un-normalized face normal
			XMStoreFloat3(&unnormalized,XMVector3Cross(edge1,edge2));

			tempNormal.push_back(unnormalized);

			//Find first texture coordinate edge 2d vector
			tcU1 = vertices[model.Indices[(i * 3)]].texCoord.x - vertices[model.Indices[(i * 3) + 2]].texCoord.x;
			tcV1 = vertices[model.Indices[(i * 3)]].texCoord.y - vertices[model.Indices[(i * 3) + 2]].texCoord.y;

			//Find second texture coordinate edge 2d vector
			tcU2 = vertices[model.Indices[(i * 3) + 2]].texCoord.x - vertices[model.Indices[(i * 3) + 1]].texCoord.x;
			tcV2 = vertices[model.Indices[(i * 3) + 2]].texCoord.y - vertices[model.Indices[(i * 3) + 1]].texCoord.y;

			//Find tangent using both tex coord edges and position edges
			tangent.x = (tcV1 * XMVectorGetX(edge1) - tcV2 * XMVectorGetX(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
			tangent.y = (tcV1 * XMVectorGetY(edge1) - tcV2 * XMVectorGetY(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
			tangent.z = (tcV1 * XMVectorGetZ(edge1) - tcV2 * XMVectorGetZ(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));

			tempTangent.push_back(tangent);
		}

		//Compute vertex normals (normal Averaging)
		XMVECTOR normalSum = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
		XMVECTOR tangentSum = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
		int facesUsing = 0;
		float tX,tY,tZ;   //temp axis variables

						  //Go through each vertex
		for(int i = 0; i < totalVerts; ++i) {
			//Check which triangles use this vertex
			for(int j = 0; j < meshTriangles; ++j) {
				if(model.Indices[j * 3] == i ||
					model.Indices[(j * 3) + 1] == i ||
					model.Indices[(j * 3) + 2] == i) {
					tX = XMVectorGetX(normalSum) + tempNormal[j].x;
					tY = XMVectorGetY(normalSum) + tempNormal[j].y;
					tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

					normalSum = XMVectorSet(tX,tY,tZ,0.0f);  //If a face is using the vertex, add the unormalized face normal to the normalSum

															 //We can reuse tX, tY, tZ to sum up tangents
					tX = XMVectorGetX(tangentSum) + tempTangent[j].x;
					tY = XMVectorGetY(tangentSum) + tempTangent[j].y;
					tZ = XMVectorGetZ(tangentSum) + tempTangent[j].z;

					tangentSum = XMVectorSet(tX,tY,tZ,0.0f); //sum up face tangents using this vertex

					facesUsing++;
				}
			}

			//Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
			normalSum = normalSum / facesUsing;
			tangentSum = tangentSum / facesUsing;

			//Normalize the normalSum vector and tangent
			normalSum = XMVector3Normalize(normalSum);
			tangentSum = XMVector3Normalize(tangentSum);

			//Store the normal and tangent in our current vertex
			vertices[i].normal.x = XMVectorGetX(normalSum);
			vertices[i].normal.y = XMVectorGetY(normalSum);
			vertices[i].normal.z = XMVectorGetZ(normalSum);

			vertices[i].tangent.x = XMVectorGetX(tangentSum);
			vertices[i].tangent.y = XMVectorGetY(tangentSum);
			vertices[i].tangent.z = XMVectorGetZ(tangentSum);

			//Clear normalSum, tangentSum and facesUsing for next vertex
			normalSum = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
			tangentSum = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
			facesUsing = 0;

		}
	}

	// Create Axis-Aligned Bounding Box (AABB)
	XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX,FLT_MAX,FLT_MAX);
	XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX,-FLT_MAX,-FLT_MAX);
	model.BoundingSphere = 0;

	for(UINT i = 0; i < model.Vertices.size(); i++) {
		// The minVertex and maxVertex will most likely not be actual vertices in the m_model, but vertices
		// that use the smallest and largest x, y, and z values from the m_model to be sure ALL vertices are
		// covered by the bounding volume

		//Get the smallest vertex 
		minVertex.x = min(minVertex.x,model.Vertices[i].x);	// Find smallest x value in m_model
		minVertex.y = min(minVertex.y,model.Vertices[i].y);	// Find smallest y value in m_model
		minVertex.z = min(minVertex.z,model.Vertices[i].z);	// Find smallest z value in m_model

															//Get the largest vertex 
		maxVertex.x = max(maxVertex.x,model.Vertices[i].x);	// Find largest x value in m_model
		maxVertex.y = max(maxVertex.y,model.Vertices[i].y);	// Find largest y value in m_model
		maxVertex.z = max(maxVertex.z,model.Vertices[i].z);	// Find largest z value in m_model
	}

	// Our AABB [0] is the min vertex and [1] is the max
	model.AABB.push_back(minVertex);
	model.AABB.push_back(maxVertex);

	// Get m_models true center
	model.Center.x = maxVertex.x + minVertex.x / 2.0f;
	model.Center.y = maxVertex.y + minVertex.y / 2.0f;
	model.Center.z = maxVertex.z + minVertex.z / 2.0f;

	// Now that we have the center, get the bounding sphere	
	for(UINT i = 0; i < model.Vertices.size(); i++) {
		float x = (model.Center.x - model.Vertices[i].x) * (model.Center.x - model.Vertices[i].x);
		float y = (model.Center.y - model.Vertices[i].y) * (model.Center.y - model.Vertices[i].y);
		float z = (model.Center.z - model.Vertices[i].z) * (model.Center.z - model.Vertices[i].z);

		// Get m_models bounding sphere
		model.BoundingSphere = max(model.BoundingSphere,(x + y + z));
	}

	// We didn't use the square root when finding the largest distance since it slows things down.
	// We can square root the answer from above to get the actual bounding sphere now
	model.BoundingSphere = sqrt(model.BoundingSphere);


	//Create index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc,sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * meshTriangles * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &model.Indices[0];
	device->CreateBuffer(&indexBufferDesc,&iinitData,&model.IndexBuff);

	//Create Vertex Buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc,sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * totalVerts;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData,sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &vertices[0];
	hr = device->CreateBuffer(&vertexBufferDesc,&vertexBufferData,&model.VertBuff);

	return true;
}


bool Model::Initialize() {

	return true;
}

void Model::Render(
	Direct3D* d3d,
	Camera* cam,
	UINT stride,
	UINT offset,
	cbPerObject& cbPerObj
) {
	d3d->GetDeviceContext()->IASetIndexBuffer(model.IndexBuff,DXGI_FORMAT_R32_UINT,0);
	// Set the grounds vertex buffer
	d3d->GetDeviceContext()->IASetVertexBuffers(0,1,&model.VertBuff,&stride,&offset);

	// Set the WVP matrix and send it to the constant buffer in effect file
	// This also only needs to be set once per model
	cam->WVP = model.World * cam->camView * cam->camProjection;
	cbPerObj.WVP = XMMatrixTranspose(cam->WVP);
	cbPerObj.World = XMMatrixTranspose(model.World);

	for(int i = 0; i < model.Subsets; ++i) {
		// Only draw the NON-transparent parts of the model. 
		if(!material[model.SubsetMaterialID[i]].IsTransparent) {
			
			cbPerObj.difColor = material[model.SubsetMaterialID[i]].Diffuse;			// Let shader know which color to draw the model (if no diffuse texture we defined)
			cbPerObj.hasTexture = material[model.SubsetMaterialID[i]].HasDiffTexture;	// Let shader know if we need to use a texture
			cbPerObj.hasNormMap = material[model.SubsetMaterialID[i]].HasNormMap;		// Let shader know if we need to do normal mapping

			d3d->GetDeviceContext()->UpdateSubresource(d3d->cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
			d3d->GetDeviceContext()->VSSetConstantBuffers(0, 1, &d3d->cbPerObjectBuffer);
			d3d->GetDeviceContext()->PSSetConstantBuffers(1, 1, &d3d->cbPerObjectBuffer);

			// If this subset has a diffuse texture, send it to the pixel shader
			if (material[model.SubsetMaterialID[i]].HasDiffTexture)
				d3d->GetDeviceContext()->PSSetShaderResources(0, 1, &textureMgr.TextureList[material[model.SubsetMaterialID[i]].DiffuseTextureID]);

			// If this subset has a normal (bump) map, send it to the pixel shader
			if (material[model.SubsetMaterialID[i]].HasNormMap)
				d3d->GetDeviceContext()->PSSetShaderResources(1, 1, &textureMgr.TextureList[material[model.SubsetMaterialID[i]].NormMapTextureID]);

			// Draw the NON-transparent stuff
			int indexStart = model.SubsetIndexStart[i];
			int indexDrawAmount = model.SubsetIndexStart[i + 1] - indexStart;

			d3d->GetDeviceContext()->DrawIndexed(indexDrawAmount,indexStart,0);
		}
	}
}

void Model::Render_gizmos(
	Direct3D* d3d,
	Camera* cam,
	UINT stride,
	UINT offset,
	cbPerObject_gizmos& cbPerObj_gizmod,
	DirectX::XMFLOAT4 color
) {
	d3d->GetDeviceContext()->IASetIndexBuffer(model.IndexBuff, DXGI_FORMAT_R32_UINT, 0);
	// Set the grounds vertex buffer
	d3d->GetDeviceContext()->IASetVertexBuffers(0, 1, &model.VertBuff, &stride, &offset);

	// Set the WVP matrix and send it to the constant buffer in effect file
	// This also only needs to be set once per model
	cam->WVP = model.World * cam->camView * cam->camProjection;
	cbPerObj_gizmod.WVP = XMMatrixTranspose(cam->WVP);
	cbPerObj_gizmod.World = XMMatrixTranspose(model.World);

	for (int i = 0; i < model.Subsets; ++i) {
		// Only draw the NON-transparent parts of the model. 
		if (!material[model.SubsetMaterialID[i]].IsTransparent) {

			cbPerObj_gizmod.color = color;

			d3d->GetDeviceContext()->UpdateSubresource(d3d->cbPerObjectBuffer_gizmos, 0, NULL, &cbPerObj_gizmod, 0, 0);
			d3d->GetDeviceContext()->VSSetConstantBuffers(0, 1, &d3d->cbPerObjectBuffer_gizmos);
			d3d->GetDeviceContext()->PSSetConstantBuffers(1, 1, &d3d->cbPerObjectBuffer_gizmos);

			// Draw the NON-transparent stuff
			int indexStart = model.SubsetIndexStart[i];
			int indexDrawAmount = model.SubsetIndexStart[i + 1] - indexStart;

			d3d->GetDeviceContext()->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}
}

void Model::Shutdown() {

}

void Model::Update() {
	model.World = XMMatrixIdentity();

	transform.MRotation = XMMatrixRotationX(transform.rotation.x);
	transform.MRotation = XMMatrixRotationY(transform.rotation.y);
	transform.MRotation = XMMatrixRotationZ(transform.rotation.z);

	transform.MScale = XMMatrixScaling(transform.scail.x,transform.scail.y,transform.scail.z);
	transform.MTranslation = XMMatrixTranslation(transform.position.x,transform.position.y,transform.position.z);

	model.World = transform.MRotation * transform.MScale * transform.MTranslation;
}