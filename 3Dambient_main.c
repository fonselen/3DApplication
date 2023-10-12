#include <stdio.h>
#include <stdlib.h>
#include <SDL.h> // Para a biblioteca SDL
#include <math.h>

#define pi 3.14159265358979323
#define EXP 2.71828182845904523536

/* Definicao das estruturas Geometricas */
typedef struct
{
	double x; 		// define a coordenada x de um ponto no espaco
	double y;		// define a coordenada y de um ponto no espaco
	double z;		// define a coordenada z de um ponto no espaco
} XYZPoint;

typedef struct
{
	XYZPoint* WorldVertexList; //Lista de vertices no espaco do mundo
	XYZPoint* CamCoordSysVertexList; // Lista de vertices no espaco da camera
	XYZPoint* ProjectionSpaceVertexList; // Lista de vertices no espaco de projecao
	char* VisibleVertices; //Lista de controle de vertices visiveis. Recebe o valor T na posicao i indicanco que o vertice i da lista e visivel.
	int* TriangleList; // Lista de triangulos no espaco do mundo
	int* PSTriangleList; //Lista de triangulos do espaço de projecao
	int Vertex_N; // numero de vertices no espaco do mundo
	int Triangle_N; // numero de triangulos no espaco do mundo
	int PSVertex_N; // numero de vertices no espaco de projecao
	int PSTriangle_N; // numero de triangulos no espaco de projecao
} XYZObject;

typedef struct
{
	XYZPoint base[3];
	XYZPoint Origin;
} XYZCoordinateSystem;

typedef struct
{
	double Data[3][3];
} XYZTransformatioMatrix;

typedef struct
{
	XYZCoordinateSystem camCS;
	double ProjectionPlane_Z;
	double NearPlane_Z;
	double FarPlane_Z;
	double ProjectioPlaneWidth;
	double ProjectionPlaneHeight;
	double WidthDepthRatio;
	double HeightDepthRatio;
	double BB_X; //Bounding Box x coordinate (-BB_X, 0) and (+BB_X, 0)
	double BB_Y; //Bounding Box y coordinate (0, -BB_y) and (0, +BB_Y)
} XYZCam;

//Funcoes de transformacao geometrica
XYZPoint XYZ_PointRotation(XYZPoint P, XYZPoint O, double ang);
XYZPoint XYZ_PointTranslation(XYZPoint P, XYZPoint Vector, double t);
XYZTransformatioMatrix XYZ_SetRotationMatrix(XYZPoint RotationAxis, double ang);
XYZPoint XYZ_ApplyTransformationMatrix(XYZPoint P, XYZTransformatioMatrix M);

//Funcoes de alocacao
void XYZ_AllocateObjectLists(XYZObject* OBJ, int N_Vertices, int N_Triangles);
void XYZ_DeallocatedObjectLists(XYZObject* OBJ);

//Funcoes de pipeline
void XYZ_SetOBJCamCoordinates(XYZObject* OBJ, XYZCam* Cam);
void XYZ_ApplyTriangleCulling(XYZObject* OBJ, XYZCam* Cam);
void XYZ_SetOBJProjectionSpaceCoordinate(XYZObject* OBJ, XYZCam* Cam);
void XYZ_ViewportOBJRender(XYZObject* OBJ, XYZCam* Cam);
void Viewport_Ground_Render(XYZPoint** V, XYZCam Cam, int NumVertices_i, int NumVertices_j);
void drawLine(SDL_Renderer* renderer, int xi, int yi, int xf, int yf);

void XYZ_SetOBJCamFrustum(XYZObject* OBJ, XYZCam* Cam);
void XYZ_SetSphericalObject(XYZPoint* S, int* V_index);


SDL_Renderer* renderer = NULL;

int main(int argc, char** argv)
{

	int i, j, k, MouseMH, MouseMV, aux_i, aux_j;
	int fat = 10;
	int GL, GC, CamViewControl = 0, CamCommandControl = 0;
	XYZCam Cam1, Cam2;
	XYZObject CubeList[300], PiramideList[300], SphereVertices, GroundSurface[100][100], CamFrustum;
	XYZPoint **GroundCoordinates;
	XYZTransformatioMatrix M, M_Esfera;
	XYZPoint P, WalkAxis[2];
	SDL_Event event;
	double RotCamAngAzimutal, RotCamAngPolar;
	XYZCoordinateSystem InertialCoordinateSystem;
	
	//XYZ_SetSphericalObject(SphereVertices, SphereIndex);
	
	InertialCoordinateSystem.Origin.x = 0;
	InertialCoordinateSystem.Origin.y = 0;
	InertialCoordinateSystem.Origin.z = 0;
	
	InertialCoordinateSystem.base[0].x = 1;
	InertialCoordinateSystem.base[0].y = 0;
	InertialCoordinateSystem.base[0].z = 0;

	InertialCoordinateSystem.base[1].x = 0;
	InertialCoordinateSystem.base[1].y = 1;
	InertialCoordinateSystem.base[1].z = 0;

	InertialCoordinateSystem.base[2].x = 0;
	InertialCoordinateSystem.base[2].y = 0;
	InertialCoordinateSystem.base[2].z = 1;

	
	GL = 5000;
	GC = 5000;

	/**************** Alocacao de parametros ****************/

	// Alocação do ponteiro duplo
	GroundCoordinates = (XYZPoint**)malloc(GL * sizeof(XYZPoint*));
	
	if (GroundCoordinates == NULL)
	{
		printf("Ground array allocation failed\n");
		return 1;
	}

	// Alocação para cada linha do ponteiro duplo
	for (int i = 0; i < GL; i++)
	{
		
		GroundCoordinates[i] = (XYZPoint*)malloc(GC * sizeof(XYZPoint));
		
		if (GroundCoordinates[i] == NULL)
		{
			printf("Ground array allocation failed %d.\n", i);
			return 1;
		}
	}

	for (i = 0; i < 300; i++)
	{
		CubeList[i].Vertex_N = 8;
		CubeList[i].Triangle_N = 12;

		XYZ_AllocateObjectLists(&CubeList[i], CubeList[i].Vertex_N, CubeList[i].Triangle_N);
	}

	for (i = 0; i < 100; i++)
	{
		for (j = 0; j < 100; j++)
		{
			GroundSurface[i][j].Vertex_N = 121;
			GroundSurface[i][j].Triangle_N = 200;

			XYZ_AllocateObjectLists(&GroundSurface[i][j], GroundSurface[i][j].Vertex_N, GroundSurface[i][j].Triangle_N);
		}
	}

	CamFrustum.Vertex_N = 13;
	CamFrustum.PSVertex_N = 13;
	CamFrustum.Triangle_N = 10;
	CamFrustum.PSTriangle_N = 10;

	XYZ_AllocateObjectLists(&CamFrustum, CamFrustum.Vertex_N, CamFrustum.Triangle_N);
	
	for (i = 0; i < 13; i++)
	{
		CamFrustum.VisibleVertices[i] = 'T';
	}

	/**************** Fim da alocacao ****************/

	for (i = 0; i < GL; i++)
	{
		for (j = 0; j < GC; j++)
		{
			GroundCoordinates[i][j].x = -GC/2 + j;
			GroundCoordinates[i][j].y = 0;// 0.5 * sin(i) * sin(j);
			GroundCoordinates[i][j].z = -GL/2 + i;
			//printf(" (%.2lf, %.2lf, %.2lf) ", GroundCoordinates[i][j].x, GroundCoordinates[i][j].y, GroundCoordinates[i][j].z);
			
		}

	}

	//Definicao manual da camera
	Cam1.camCS.base[0].x = 1;
	Cam1.camCS.base[0].y = 0;
	Cam1.camCS.base[0].z = 0;

	Cam1.camCS.base[1].x = 0;
	Cam1.camCS.base[1].y = 1;
	Cam1.camCS.base[1].z = 0;

	Cam1.camCS.base[2].x = 0;
	Cam1.camCS.base[2].y = 0;
	Cam1.camCS.base[2].z = 1;

	Cam1.camCS.Origin.x = 0;
	Cam1.camCS.Origin.y = 200;
	Cam1.camCS.Origin.z = 0;

	Cam1.ProjectionPlane_Z = 0.5;
	Cam1.NearPlane_Z = 2;
	Cam1.FarPlane_Z = 20;
	Cam1.ProjectioPlaneWidth = 0.5;
	Cam1.ProjectionPlaneHeight = 0.28;
	Cam1.WidthDepthRatio = (Cam1.ProjectioPlaneWidth / 2) / Cam1.ProjectionPlane_Z;
	Cam1.HeightDepthRatio = (Cam1.ProjectionPlaneHeight / 2) / Cam1.ProjectionPlane_Z;




	Cam2 = Cam1;
	Cam2.camCS.Origin.y = 0.5;
	Cam1.camCS.Origin.z = -10;

/*
	Cam1.camCS.base[2].x = 0;
	Cam1.camCS.base[2].y = -1;
	Cam1.camCS.base[2].z = 0;
*/
	WalkAxis[0] = Cam1.camCS.base[2];
	WalkAxis[1] = Cam2.camCS.base[2];

	P.x = 0;
	P.y = 0;
	P.z = 30;
	j = 0;

	for (i = 0; i < 300; i++)
	{
		if (j == 10)
		{
			//P.x += 2;
			//j = 0;
		}

		P.x = (rand() % 100) - 50;
		P.y = (rand() % 50);
		P.z = (rand() % 100) - 50;


		CubeList[i].WorldVertexList[0].x = P.x;
		CubeList[i].WorldVertexList[0].y = P.y;
		CubeList[i].WorldVertexList[0].z = P.z + 2 * j;
		CubeList[i].WorldVertexList[1].x = P.x + 1;
		CubeList[i].WorldVertexList[1].y = P.y;
		CubeList[i].WorldVertexList[1].z = P.z + 2 * j;
		CubeList[i].WorldVertexList[2].x = P.x + 1;
		CubeList[i].WorldVertexList[2].y = P.y + 1;
		CubeList[i].WorldVertexList[2].z = P.z + 2 * j;
		CubeList[i].WorldVertexList[3].x = P.x;
		CubeList[i].WorldVertexList[3].y = P.y + 1;
		CubeList[i].WorldVertexList[3].z = P.z + 2 * j;

		CubeList[i].WorldVertexList[4].x = P.x;
		CubeList[i].WorldVertexList[4].y = P.y;
		CubeList[i].WorldVertexList[4].z = P.z + 1 + 2 * j;
		CubeList[i].WorldVertexList[5].x = P.x + 1;
		CubeList[i].WorldVertexList[5].y = P.y;
		CubeList[i].WorldVertexList[5].z = P.z + 1 + 2 * j;
		CubeList[i].WorldVertexList[6].x = P.x + 1;
		CubeList[i].WorldVertexList[6].y = P.y + 1;
		CubeList[i].WorldVertexList[6].z = P.z + 1 + 2 * j;
		CubeList[i].WorldVertexList[7].x = P.x;
		CubeList[i].WorldVertexList[7].y = P.y + 1;
		CubeList[i].WorldVertexList[7].z = P.z + 1 + 2 * j;



		CubeList[i].TriangleList[0] = 0;
		CubeList[i].TriangleList[1] = 1;
		CubeList[i].TriangleList[2] = 2;

		CubeList[i].TriangleList[3] = 0;
		CubeList[i].TriangleList[4] = 3;
		CubeList[i].TriangleList[5] = 2;

		CubeList[i].TriangleList[6] = 4;
		CubeList[i].TriangleList[7] = 5;
		CubeList[i].TriangleList[8] = 6;

		CubeList[i].TriangleList[9] = 4;
		CubeList[i].TriangleList[10] = 7;
		CubeList[i].TriangleList[11] = 6;

		CubeList[i].TriangleList[12] = 4;
		CubeList[i].TriangleList[13] = 0;
		CubeList[i].TriangleList[14] = 3;

		CubeList[i].TriangleList[15] = 3;
		CubeList[i].TriangleList[16] = 7;
		CubeList[i].TriangleList[17] = 4;

		CubeList[i].TriangleList[18] = 2;
		CubeList[i].TriangleList[19] = 1;
		CubeList[i].TriangleList[20] = 5;

		CubeList[i].TriangleList[21] = 6;
		CubeList[i].TriangleList[22] = 2;
		CubeList[i].TriangleList[23] = 3;

		CubeList[i].TriangleList[24] = 6;
		CubeList[i].TriangleList[25] = 7;
		CubeList[i].TriangleList[26] = 3;

		CubeList[i].TriangleList[27] = 6;
		CubeList[i].TriangleList[28] = 2;
		CubeList[i].TriangleList[29] = 3;

		CubeList[i].TriangleList[30] = 5;
		CubeList[i].TriangleList[31] = 4;
		CubeList[i].TriangleList[32] = 0;

		CubeList[i].TriangleList[33] = 0;
		CubeList[i].TriangleList[34] = 1;
		CubeList[i].TriangleList[35] = 5;

		//j++;

	}


	// Definicao da superficie do chao
	for (i = 0; i < 100; i++)
	{
		for (j = 0; j < 100; j++)
		{
			GroundSurface[i][j].WorldVertexList[0].x = -550 + 11 * i;
			GroundSurface[i][j].WorldVertexList[0].y = 0;
			GroundSurface[i][j].WorldVertexList[0].z = -550 + 11 * j;
			
			aux_j = 0;

			for (k = 1; k < 121; k++)
			{
				if (GroundSurface[i][j].WorldVertexList[k - 1].x < -550 + i * 11 + 9.9)
				{
					GroundSurface[i][j].WorldVertexList[k].x = GroundSurface[i][j].WorldVertexList[k - 1].x + 1;
					GroundSurface[i][j].WorldVertexList[k].y = 0;
					GroundSurface[i][j].WorldVertexList[k].z = GroundSurface[i][j].WorldVertexList[0].z + aux_j;
				}

				else
				{
					GroundSurface[i][j].WorldVertexList[k].x = -550 + 11 * i;
					GroundSurface[i][j].WorldVertexList[k].y = 0;
					aux_j ++;
					GroundSurface[i][j].WorldVertexList[k].z = GroundSurface[i][j].WorldVertexList[0].z + aux_j;
				}

			}

		}
	}


	j = 0;
	for (i = 0; i < 109; i++)
	{
		if (GroundSurface[0][0].WorldVertexList[i].x < -550.0 + 9.9)
		{
			GroundSurface[0][0].TriangleList[6 * j] = i;
			GroundSurface[0][0].TriangleList[6 * j + 1] = i + 1;
			GroundSurface[0][0].TriangleList[6 * j + 2] = i + 12;
			GroundSurface[0][0].TriangleList[6 * j + 3] = i;
			GroundSurface[0][0].TriangleList[6 * j + 4] = i + 11;
			GroundSurface[0][0].TriangleList[6 * j + 5] = i + 12;
			j++;
		}

	}

	for (i = 0; i < 100; i++)
	{
		for (j = 0; j < 100; j++)
		{
			for (k = 0; k < 600; k++)
			{
				GroundSurface[i][j].TriangleList[k] = GroundSurface[0][0].TriangleList[k];
			}
		}
	}





	// Inicializa o SDL
	SDL_Init(SDL_INIT_VIDEO);

	// Cria uma janela
	SDL_Window* window = SDL_CreateWindow("Desenhar Linha com SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 2500, 1400, SDL_WINDOW_SHOWN);

	// Cria um renderer para a janela
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);


	k = 0;
	SDL_SetWindowGrab(window, SDL_TRUE);
	
	SDL_SetRelativeMouseMode(SDL_TRUE);
	
	while (k == 0)
	{
		while (SDL_PollEvent(&event))
		{
			

			if (event.type == SDL_MOUSEMOTION)
			{
				MouseMH = event.motion.xrel;
				MouseMV = event.motion.yrel;

				RotCamAngAzimutal = (MouseMH * pi / 180) / 10;
				RotCamAngPolar = (MouseMV * pi / 180) / 10;
				//printf("\n Azimutal = %lf - Polar = %lf", RotCamAngAzimutal, RotCamAngPolar);

				M = XYZ_SetRotationMatrix(InertialCoordinateSystem.base[1], RotCamAngAzimutal);

				if (CamCommandControl == 0)
				{
					Cam1.camCS.base[0] = XYZ_ApplyTransformationMatrix(Cam1.camCS.base[0], M);
					Cam1.camCS.base[1] = XYZ_ApplyTransformationMatrix(Cam1.camCS.base[1], M);
					Cam1.camCS.base[2] = XYZ_ApplyTransformationMatrix(Cam1.camCS.base[2], M);
					WalkAxis[0] = XYZ_ApplyTransformationMatrix(WalkAxis[0], M);

					M = XYZ_SetRotationMatrix(Cam1.camCS.base[0], RotCamAngPolar);

					Cam1.camCS.base[1] = XYZ_ApplyTransformationMatrix(Cam1.camCS.base[1], M);
					Cam1.camCS.base[2] = XYZ_ApplyTransformationMatrix(Cam1.camCS.base[2], M);
				}

				if (CamCommandControl == 1)
				{
					Cam2.camCS.base[0] = XYZ_ApplyTransformationMatrix(Cam2.camCS.base[0], M);
					Cam2.camCS.base[1] = XYZ_ApplyTransformationMatrix(Cam2.camCS.base[1], M);
					Cam2.camCS.base[2] = XYZ_ApplyTransformationMatrix(Cam2.camCS.base[2], M);
					WalkAxis[1] = XYZ_ApplyTransformationMatrix(WalkAxis[1], M);

					M = XYZ_SetRotationMatrix(Cam2.camCS.base[0], RotCamAngPolar);

					Cam2.camCS.base[1] = XYZ_ApplyTransformationMatrix(Cam2.camCS.base[1], M);
					Cam2.camCS.base[2] = XYZ_ApplyTransformationMatrix(Cam2.camCS.base[2], M);
				}

			}

			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE) k = 1;


				if (event.key.keysym.sym == SDLK_z)
				{
					RotCamAngAzimutal = 0.01;
					Cam2.camCS.base[0] = XYZ_PointRotation(Cam2.camCS.base[0], Cam2.camCS.base[0], RotCamAngAzimutal);
					Cam2.camCS.base[2] = XYZ_PointRotation(Cam2.camCS.base[2], Cam2.camCS.base[2], RotCamAngAzimutal);

				}

				

				if (event.key.keysym.sym == SDLK_w)
				{
					Cam1.camCS.Origin = XYZ_PointTranslation(Cam1.camCS.Origin, WalkAxis[0], 0.2);

				}

				if (event.key.keysym.sym == SDLK_s)
				{
					Cam1.camCS.Origin = XYZ_PointTranslation(Cam1.camCS.Origin, WalkAxis[0], -0.2);

				}

				if (event.key.keysym.sym == SDLK_d)
				{
					Cam1.camCS.Origin = XYZ_PointTranslation(Cam1.camCS.Origin, Cam1.camCS.base[0], 0.1);

				}

				if (event.key.keysym.sym == SDLK_a)
				{
					Cam1.camCS.Origin = XYZ_PointTranslation(Cam1.camCS.Origin, Cam1.camCS.base[0], -0.1);

				}

				if (event.key.keysym.sym == SDLK_UP)
				{
					Cam1.camCS.Origin = XYZ_PointTranslation(Cam1.camCS.Origin, InertialCoordinateSystem.base[1], 0.3);

				}

				if (event.key.keysym.sym == SDLK_DOWN)
				{
					Cam1.camCS.Origin = XYZ_PointTranslation(Cam1.camCS.Origin, InertialCoordinateSystem.base[1], -0.3);

				}


				if (event.key.keysym.sym == SDLK_v)
				{
					if (CamViewControl == 0) CamViewControl = 1;
					else CamViewControl = 0;
				}

				if (event.key.keysym.sym == SDLK_c)
				{
					if (CamCommandControl == 0) CamCommandControl = 1;
					else CamCommandControl = 0;

				}



			}
		}
	
		

		if (CamViewControl == 0)
		{

			aux_i = (int)((Cam2.camCS.Origin.x + 550.0) / 11);
			aux_j = (int)((Cam2.camCS.Origin.z + 550.0) / 11);

			i = aux_i - 5;
			j = aux_j - 5;

			if (i < 0) i = 0;
			if (j < 0) j = 0;

			while (i < (aux_i + 5) && i < 100)
			{
				while (j < (aux_j + 5) && j < 100)
				{
					XYZ_SetOBJCamCoordinates(&GroundSurface[i][j], &Cam2);
					XYZ_ApplyTriangleCulling(&GroundSurface[i][j], &Cam2);
					XYZ_SetOBJProjectionSpaceCoordinate(&GroundSurface[i][j], &Cam2);
					XYZ_ViewportOBJRender(&GroundSurface[i][j], &Cam2);

					j++;
				}

				i++;
				j = aux_j - 5;
			}


			SDL_SetRenderDrawColor(renderer, 50, 50, 250, 255);

			for (i = 0; i < 300; i++)
			{
				XYZ_SetOBJCamCoordinates(&CubeList[i], &Cam2);
				XYZ_ApplyTriangleCulling(&CubeList[i], &Cam2);
				XYZ_SetOBJProjectionSpaceCoordinate(&CubeList[i], &Cam2);
				XYZ_ViewportOBJRender(&CubeList[i], &Cam2);
			}

			
		}


		if (CamViewControl == 1)
		{
			XYZ_SetOBJCamFrustum(&CamFrustum, &Cam2);
			aux_i = (int)((Cam2.camCS.Origin.x + 550.0) / 11);
			aux_j = (int)((Cam2.camCS.Origin.z + 550.0) / 11);

			i = aux_i - 5;
			j = aux_j - 5;

			if (i < 0) i = 0;
			if (j < 0) j = 0;

			while (i < (aux_i + 5) && i < 100)
			{
				while (j < (aux_j + 5) && j < 100)
				{
					XYZ_SetOBJCamCoordinates(&GroundSurface[i][j], &Cam2);
					XYZ_ApplyTriangleCulling(&GroundSurface[i][j], &Cam2);
					XYZ_SetOBJCamCoordinates(&GroundSurface[i][j], &Cam1);
					XYZ_SetOBJProjectionSpaceCoordinate(&GroundSurface[i][j], &Cam1);
					XYZ_ViewportOBJRender(&GroundSurface[i][j], &Cam1);

					j++;
				}

				i++;
				j = aux_j - 5;
			}


			SDL_SetRenderDrawColor(renderer, 50, 50, 250, 255);

			for (i = 0; i < 300; i++)
			{
				XYZ_SetOBJCamCoordinates(&CubeList[i], &Cam2);
				XYZ_ApplyTriangleCulling(&CubeList[i], &Cam2);
				XYZ_SetOBJCamCoordinates(&CubeList[i], &Cam1);
				XYZ_SetOBJProjectionSpaceCoordinate(&CubeList[i], &Cam1);
				XYZ_ViewportOBJRender(&CubeList[i], &Cam1);
			}

			//XYZ_SetOBJCamCoordinates(&CamFrustum, &Cam2);
			//XYZ_ApplyTriangleCulling(&CamFrustum, &Cam2);
			XYZ_SetOBJCamCoordinates(&CamFrustum, &Cam1);
			XYZ_SetOBJProjectionSpaceCoordinate(&CamFrustum, &Cam1);
			XYZ_ViewportOBJRender(&CamFrustum, &Cam1);
		}

		

		
				
		

		//delay(40);
		//SDL_Delay(10);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		drawLine(renderer, 1248, 700, 1252, 700);
		drawLine(renderer, 1250, 698, 1250, 702);
		SDL_RenderPresent(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		//GroundShow(Cam1, 0.5, 0, 0, 0, 0);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	}
	
	
	SDL_SetWindowGrab(window, SDL_FALSE);

	for (int i = 0; i < GL; i++) 
	{
		free(GroundCoordinates[i]);
	}
	
	free(GroundCoordinates);

	for (i = 0; i < 300; i++)
	{ 
		XYZ_DeallocatedObjectLists(&CubeList[i]);
	}

	
	for (i = 0; i < 100; i++)
	{
		for (j = 0; j < 100; j++)
		{
			XYZ_DeallocatedObjectLists(&GroundSurface[i][j]);
		}
	}


	XYZ_DeallocatedObjectLists(&CamFrustum);


	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}


XYZPoint XYZ_PointRotation(XYZPoint P, XYZPoint O, double ang)
{
	XYZPoint Paux;

	Paux = P;

	Paux.x = P.x * cos(ang) - P.z * sin(ang);
	Paux.z = P.x * sin(ang) + P.z * cos(ang);

	return Paux;
}

XYZPoint XYZ_PointTranslation(XYZPoint P, XYZPoint Vector, double t)
{
	XYZPoint Paux;

	Paux = P;

	Paux.x = P.x + t * Vector.x;
	Paux.y = P.y + t * Vector.y;
	Paux.z = P.z + t * Vector.z;

	return Paux;
}


XYZTransformatioMatrix XYZ_SetRotationMatrix(XYZPoint RotationAxis, double ang)
{
	XYZTransformatioMatrix RotMatT;
	double I[3][3], J[3][3], JJ[3][3];
	int i, j;

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (i == j)
			{
				I[i][j] = 1;
				J[i][j] = 0;
			}
			else
			{
				I[i][j] = 0;
			}
		}
	}

	J[0][1] = -RotationAxis.z;
	J[0][2] = RotationAxis.y;
	J[1][0] = RotationAxis.z;
	J[1][2] = -RotationAxis.x;
	J[2][0] = -RotationAxis.y;
	J[2][1] = RotationAxis.x;

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			JJ[i][j] = J[i][0] * J[0][j] + J[i][1] * J[1][j] + J[i][2] * J[2][j];

			RotMatT.Data[i][j] = I[i][j] + ((sin(ang)) * J[i][j]) + ((1 - cos(ang)) * JJ[i][j]);
		}
	}

	return RotMatT;
}

XYZPoint XYZ_ApplyTransformationMatrix(XYZPoint P, XYZTransformatioMatrix M)
{
	XYZPoint Q;

	Q.x = M.Data[0][0] * P.x + M.Data[0][1] * P.y + M.Data[0][2] * P.z;
	Q.y = M.Data[1][0] * P.x + M.Data[1][1] * P.y + M.Data[1][2] * P.z;
	Q.z = M.Data[2][0] * P.x + M.Data[2][1] * P.y + M.Data[2][2] * P.z;

	return Q;
}

void XYZ_AllocateObjectLists(XYZObject* OBJ, int N_Vertices, int N_Triangles)
{
	(*OBJ).WorldVertexList = (XYZPoint*)malloc(N_Vertices * sizeof(XYZPoint));
	(*OBJ).CamCoordSysVertexList = (XYZPoint*)malloc(N_Vertices * sizeof(XYZPoint));
	(*OBJ).ProjectionSpaceVertexList = (XYZPoint*)malloc(N_Vertices * sizeof(XYZPoint));
	(*OBJ).VisibleVertices = (int*)malloc(N_Vertices * sizeof(int));
	(*OBJ).TriangleList = (int*)malloc( (3 * N_Triangles) * sizeof(int));
	(*OBJ).PSTriangleList = (int*)malloc((3 * N_Triangles) * sizeof(int));

	if ((*OBJ).WorldVertexList == NULL)
	{
		printf("Vertex list allocation failed");
		exit(1);
	}

	if ((*OBJ).CamCoordSysVertexList == NULL)
	{
		printf("Vertex list allocation failed");
		exit(1);
	}

	if ((*OBJ).ProjectionSpaceVertexList == NULL)
	{
		printf("Vertex list allocation failed");
		exit(1);
	}
	
	if ((*OBJ).VisibleVertices == NULL)
	{
		printf("Visible Vertex list allocation failed");
		exit(1);
	}

	if ((*OBJ).TriangleList == NULL)
	{
		printf("Triangle list allocation failed");
		exit(1);
	}

	if ((*OBJ).PSTriangleList == NULL)
	{
		printf("Triangle list allocation failed");
		exit(1);
	}
}

void XYZ_DeallocatedObjectLists(XYZObject* OBJ)
{
	free((*OBJ).WorldVertexList);
	free((*OBJ).CamCoordSysVertexList);
	free((*OBJ).ProjectionSpaceVertexList);
	free((*OBJ).VisibleVertices);
	free((*OBJ).TriangleList);
	free((*OBJ).PSTriangleList);


	(*OBJ).WorldVertexList = NULL;
	(*OBJ).TriangleList = NULL;
}


/************************   Area atualmente em desenvolvimento  *******************************************/

void XYZ_SetOBJCamCoordinates(XYZObject* OBJ, XYZCam* Cam)
{
	int i;
	XYZPoint Paux;

	for (i = 0; i < (*OBJ).Vertex_N; i++)
	{
		Paux = (*OBJ).WorldVertexList[i];

		Paux.x = Paux.x - (*Cam).camCS.Origin.x;
		Paux.y = Paux.y - (*Cam).camCS.Origin.y;
		Paux.z = Paux.z - (*Cam).camCS.Origin.z;

		(*OBJ).CamCoordSysVertexList[i].x = (*Cam).camCS.base[0].x * Paux.x + (*Cam).camCS.base[0].y * Paux.y + (*Cam).camCS.base[0].z * Paux.z;
		(*OBJ).CamCoordSysVertexList[i].y = (*Cam).camCS.base[1].x * Paux.x + (*Cam).camCS.base[1].y * Paux.y + (*Cam).camCS.base[1].z * Paux.z;
		(*OBJ).CamCoordSysVertexList[i].z = (*Cam).camCS.base[2].x * Paux.x + (*Cam).camCS.base[2].y * Paux.y + (*Cam).camCS.base[2].z * Paux.z;
	}
}

void XYZ_ApplyTriangleCulling(XYZObject* OBJ, XYZCam* Cam)
{
	XYZPoint PA, PB, PC, Paux, normal;
	int i, j = 0, TriangleListLength;

	TriangleListLength = (3 * (*OBJ).Triangle_N);

	for (i = 0; i < (*OBJ).Vertex_N; i++)
	{
		(*OBJ).VisibleVertices[i] = 'F';
	}

	(*OBJ).PSVertex_N = 0; // zera o numero de vertices no espaco de projecao
	(*OBJ).PSTriangle_N = 0; // zera o numero de triangulos no espaco de projecao

	for (i = 0; i < TriangleListLength; i += 3)
	{
		PA = (*OBJ).CamCoordSysVertexList[(*OBJ).TriangleList[i]];
		PB = (*OBJ).CamCoordSysVertexList[(*OBJ).TriangleList[i + 1]];
		PC = (*OBJ).CamCoordSysVertexList[(*OBJ).TriangleList[i + 2]];

		//Culling de Frustum (deve ser paralelizado)

		//Verifica se esta entre os planos proximo ou distante
		if ((PA.z > (*Cam).FarPlane_Z && PB.z > (*Cam).FarPlane_Z && PC.z > (*Cam).FarPlane_Z) || (PA.z < (*Cam).NearPlane_Z && PB.z < (*Cam).NearPlane_Z && PC.z < (*Cam).NearPlane_Z))
		{
			continue;
		}

		
		// WidthDepthRatio define o coeficiente angular da reta definida pela interseccao do plano direito do frustum com o plano xz.
		/*
		==============================================================================================================================

		         ^ (eixo z)
		         |
				 |
				 |
		\        |        /  <-- Interseccao dos Planos laterais do frustum com o plano xz (vista superior).
		 \       |       /       A variavel WidthDepthRatio armazena o coeficiente angular dessa reta.
		  \      |      /        Com isso, fica facil determinar se um ponto esta entre os dois planos (dentro do V)
		   \     |     /         ou se está fora
		    \    |    /
			 \   |   /
			  \  |  /
			   \ | /
			    \|/
		---------*-----------------------------------> (eixo x)
		         |
		         | 

		==============================================================================================================================
		*/
		
		// Como, nesse caso, a coordenada y e irrelevante, o calculo para determinar de qual lado do plano o ponto esta pode ser reduzido ao plano xz
		// Sera analogo para os outros planos 

		// Verificacao para o plano esquerdo
		if (PA.x < (-(*Cam).WidthDepthRatio * PA.z) && PB.x < (-(*Cam).WidthDepthRatio * PB.z) && PC.x < (-(*Cam).WidthDepthRatio * PC.z)) continue;
		
		// Verificacao para o plano Direito
		if (PA.x > ((*Cam).WidthDepthRatio * PA.z) && PB.x > ((*Cam).WidthDepthRatio * PB.z) && PC.x > ((*Cam).WidthDepthRatio * PC.z)) continue;

		// Verificacao para o plano superior
		if (PA.y > ((*Cam).HeightDepthRatio * PA.z) && PB.y > ((*Cam).HeightDepthRatio * PB.z) && PC.y > ((*Cam).HeightDepthRatio * PC.z)) continue;

		// Verificacao para o plano inferior
		if (PA.y < (-(*Cam).HeightDepthRatio * PA.z) && PB.y < (-(*Cam).HeightDepthRatio * PB.z) && PC.y < (-(*Cam).HeightDepthRatio * PC.z)) continue;


		// Se o triangulo passou por todos os testes, entao o triangulo esta dentro do frustum e voltado para tela.
		//Sendo assim, ele deve ser colocado na lista de triangulos do espaco de projecao e seus vertices deverao ser transformados

		//Aqui a lista de triangulos que estao dentro do frustum e gerada. Esses serao os triangulos do espaco de projecao.
		// o indice i percorre a lista de triangulos original e o indice j percorre a lista de triangulos do espaco de projecao
		(*OBJ).PSTriangleList[j] = (*OBJ).TriangleList[i]; //O i-esimo triangulo da lista original devera entrar como o j-esimo triangulo da lista de triangulos do espaco de projecao
		(*OBJ).PSTriangleList[j + 1] = (*OBJ).TriangleList[i + 1];
		(*OBJ).PSTriangleList[j + 2] = (*OBJ).TriangleList[i + 2];
		j += 3;
		//printf("\nO triangulo %d apareceu", i/3);

		/*
		==================================== Descricao do controle de vertices visiveis ====================================
		Os indices dos vertices visiveis serao salvos no array unidimensional (*OBJ).VisibleVertices. A cada tres posicoes 
		consecutivas tem-se um triangulo.
		O array com os vertices visiveis mapeia as posicoes dos vertices originais para transforma-los nas coordenadas da tela.
		Teremos entao: 
		Lista de vertices no espaco da camera: (*OBJ).CamCoordSysVertexList
		Lista de triangulo no espaco da camera: (*OBJ).TriangleList
		Lista de Vertices no espaco de projecao: (*OBJ).ProjectionSpaceVertexList
		Lista de triangulos no espaco de projecao: (*OBJ).PSTriangleList
		Lista de vertices visiveis: (*OBJ).VisibleVertices

		Se o controle de vertices na posicao i e true, entao o vertice da posicao i e visivel, caso contrario, ele e invisivel.

		Segue um exemplo de execucao:

		Array de vertices no espaco da camera (CamCoordSysVertexList)
		| Vertex1 | Vertex2 | Vertex3 | Vertex4 | Vertex5 | Vertex6 | Vertex7 | Vertex8 | Vertex9 | Vertex10 | Vertex11 | Vertex12 | Vertex13 | Vertex14 | Vertex15 |
		     0         1         2         3         4         5         6         7         8         9          10         11          12         13        14

		Array de triangulos no espaco da camera ((*OBJ).TriangleList) (eh a mesma lista para o espaco do mundo)
		| 4 | 1 | 5 | 10 | 12 | 7 |  4 | 3 | 5 | 0 | 2 | 7 |  1 | 3 | 5 | 10 | 12 | 9 | 3 | 2 | 5 | 10 | 14 | 6 |  8 | 9 | 11 | 13 | 7 | 14 |  10 | 11 | 12 | 1 | 5 | 7 | 


		Array de controle de vertices visiveis ((*OBJ).VisibleVertices) 
		Esse e o estado inicial, indica que nenhum vertice esta visivel.
		| false | false | false | false | false | false | false | false | false | false | false | false | false | false | false |
			 0      1       2       3       4       5       6       7       8       9       10      11      12      13      14

		Sao 12 triangulos. Suponha que sejam visieis apenas os seguintes triangulos
		(*OBJ).TriangleList[3]  
		(*OBJ).TriangleList[4]
		(*OBJ).TriangleList[5]

		(*OBJ).TriangleList[12]
		(*OBJ).TriangleList[13]
		(*OBJ).TriangleList[14]

		(*OBJ).TriangleList[30]
		(*OBJ).TriangleList[31]
		(*OBJ).TriangleList[32]

		Sendo assim, os vertices visiveis serao:
		(*OBJ).VisibleVertices[10] = true;
		(*OBJ).VisibleVertices[12] = true;
		(*OBJ).VisibleVertices[7] = true;

		(*OBJ).VisibleVertices[1] = true;
		(*OBJ).VisibleVertices[3] = true;
		(*OBJ).VisibleVertices[5] = true;

		(*OBJ).VisibleVertices[10] = true;
		(*OBJ).VisibleVertices[11] = true;
		(*OBJ).VisibleVertices[12] = true;

		O array de controle de vertices visiveis ((*OBJ).VisibleVertices) vai ter a seguinte configuracao:
		| false | true | false | true | false | true | false | true | false | false | true | true | true | false | false |
			 0      1      2       3      4       5      6       7      8       9      10      11    12      13      14

		==============================================================================================================================
		
		*/

		// triangulo visivel! O contador deve ser incrementado.
		(*OBJ).PSTriangle_N += 1;

		//Marca o Vertice da posicao (*OBJ).TriangleList[i] (e as duas seguintes) como visivel, pois o trianglo foi considerado visivel.
		(*OBJ).VisibleVertices[(*OBJ).TriangleList[i]] = 'T';
		(*OBJ).VisibleVertices[(*OBJ).TriangleList[i + 1]] = 'T';
		(*OBJ).VisibleVertices[(*OBJ).TriangleList[i + 2]] = 'T';

	}
}

void XYZ_SetOBJProjectionSpaceCoordinate(XYZObject* OBJ, XYZCam* Cam)
{
	int i;
	

	for (i = 0; i < (*OBJ).Vertex_N; i++)
	{
		if ((*OBJ).VisibleVertices[i] == 'T')
		{
			//Transformacao do ponto do espaco da camera para o espaco de projecao
			(*OBJ).ProjectionSpaceVertexList[i].x = ((*Cam).ProjectionPlane_Z / ((*OBJ).CamCoordSysVertexList[i].z)) * (*OBJ).CamCoordSysVertexList[i].x;
			(*OBJ).ProjectionSpaceVertexList[i].y = ((*Cam).ProjectionPlane_Z / ((*OBJ).CamCoordSysVertexList[i].z)) * (*OBJ).CamCoordSysVertexList[i].y;
			(*OBJ).ProjectionSpaceVertexList[i].z = (*Cam).ProjectionPlane_Z;
		}
		
	}

}

void XYZ_ViewportOBJRender(XYZObject* OBJ, XYZCam* Cam)
{
	XYZPoint PA, PB, PC;
	int i;
	int TriangleListLength;
	double Pixel_Width_Ratio, Pixel_Height_Ratio;

	Pixel_Width_Ratio = 2500 / (*Cam).ProjectioPlaneWidth;
	Pixel_Height_Ratio = 1400 / (*Cam).ProjectionPlaneHeight;

	// variavel criada para otimizacao
	TriangleListLength = 3 * (*OBJ).PSTriangle_N;

	for (i = 0; i < TriangleListLength; i += 3)
	{
		PA = (*OBJ).ProjectionSpaceVertexList[(*OBJ).PSTriangleList[i]];
		PB = (*OBJ).ProjectionSpaceVertexList[(*OBJ).PSTriangleList[i + 1]];
		PC = (*OBJ).ProjectionSpaceVertexList[(*OBJ).PSTriangleList[i + 2]];

		PA.x = Pixel_Width_Ratio * PA.x + 1250;
		PB.x = Pixel_Width_Ratio * PB.x + 1250;
		PC.x = Pixel_Width_Ratio * PC.x + 1250;

		PA.y = (-Pixel_Height_Ratio) * PA.y + 700;
		PB.y = (-Pixel_Height_Ratio) * PB.y + 700;
		PC.y = (-Pixel_Height_Ratio) * PC.y + 700;
		
		drawLine(renderer, PA.x, PA.y, PB.x, PB.y);
		//drawLine(renderer, PA.x, PA.y, PC.x, PC.y);
		drawLine(renderer, PB.x, PB.y, PC.x, PC.y);


	}
}

/************************ Fim da area atualmente em desenvolvimento  ************************************/


void XYZ_SetCamera(XYZCam* Cam, double x, double y, double z, double YawAng, double PitchAng, double RollAng, double ZFarPlane, double ProjectionPlaneWidth, double ProjectionPlaneHeight)
{
	//Define as coordenadas da camera dentro do sistema de coordenadas do mundo
	(*Cam).camCS.Origin.x = x;
	(*Cam).camCS.Origin.y = y;
	(*Cam).camCS.Origin.z = z;

	//Define os vetores da base do sistema de coordenadas da camera em funcao dos angulos de orientac/ao
	//os angulos serao considerados em coordenadas esfericas e definirao a orientacao do eixo z
	//vetor f2 (terceiro vetor da base)
	/*(*Cam).camCS.base[2].x = sin(ViewPolarAng) * cos(ViewAzimuthalAng);
	(*Cam).camCS.base[2].y = sin(ViewPolarAng) * sin(ViewAzimuthalAng);
	(*Cam).camCS.base[2].z = cos(ViewPolarAng);
	//Eixo f0 (primeiro vetor da base)
	(*Cam).camCS.base[0].x = sin(ViewPolarAng) * cos(ViewAzimuthalAng);
	(*Cam).camCS.base[0].y = sin(ViewPolarAng) * sin(ViewAzimuthalAng);
	(*Cam).camCS.base[0].z = cos(ViewPolarAng);
	//Eixo f1 (segundo vetor da base)
	(*Cam).camCS.base[1].x = sin(ViewPolarAng) * cos(ViewAzimuthalAng);
	(*Cam).camCS.base[1].y = sin(ViewPolarAng) * sin(ViewAzimuthalAng);
	(*Cam).camCS.base[1].z = cos(ViewPolarAng);*/

	//Define






}

void XYZ_SetOBJCamFrustum(XYZObject* OBJ, XYZCam* Cam)
{
	int i;
	XYZPoint Paux;
	double ratio;

	(*OBJ).WorldVertexList[0].x = 0;
	(*OBJ).WorldVertexList[0].y = 0;
	(*OBJ).WorldVertexList[0].z = 0;

	(*OBJ).WorldVertexList[1].x = -(*Cam).ProjectioPlaneWidth / 2;
	(*OBJ).WorldVertexList[1].y = -(*Cam).ProjectionPlaneHeight / 2;
	(*OBJ).WorldVertexList[1].z = (*Cam).ProjectionPlane_Z;

	(*OBJ).WorldVertexList[2].x = -(*Cam).ProjectioPlaneWidth / 2;
	(*OBJ).WorldVertexList[2].y = (*Cam).ProjectionPlaneHeight / 2;
	(*OBJ).WorldVertexList[2].z = (*Cam).ProjectionPlane_Z;

	(*OBJ).WorldVertexList[3].x = (*Cam).ProjectioPlaneWidth / 2;
	(*OBJ).WorldVertexList[3].y = (*Cam).ProjectionPlaneHeight / 2;
	(*OBJ).WorldVertexList[3].z = (*Cam).ProjectionPlane_Z;

	(*OBJ).WorldVertexList[4].x = (*Cam).ProjectioPlaneWidth / 2;
	(*OBJ).WorldVertexList[4].y = -(*Cam).ProjectionPlaneHeight / 2;
	(*OBJ).WorldVertexList[4].z = (*Cam).ProjectionPlane_Z;


	ratio = (*Cam).NearPlane_Z / (*Cam).ProjectionPlane_Z;

	(*OBJ).WorldVertexList[5].x = -(ratio * (*Cam).ProjectioPlaneWidth) / 2;
	(*OBJ).WorldVertexList[5].y = -(ratio * (*Cam).ProjectionPlaneHeight) / 2;
	(*OBJ).WorldVertexList[5].z = ratio * (*Cam).ProjectionPlane_Z;

	(*OBJ).WorldVertexList[6].x = -(ratio * (*Cam).ProjectioPlaneWidth) / 2;
	(*OBJ).WorldVertexList[6].y = (ratio * (*Cam).ProjectionPlaneHeight) / 2;
	(*OBJ).WorldVertexList[6].z = ratio * (*Cam).ProjectionPlane_Z;

	(*OBJ).WorldVertexList[7].x = (ratio * (*Cam).ProjectioPlaneWidth) / 2;
	(*OBJ).WorldVertexList[7].y = (ratio * (*Cam).ProjectionPlaneHeight) / 2;
	(*OBJ).WorldVertexList[7].z = ratio * (*Cam).ProjectionPlane_Z;

	(*OBJ).WorldVertexList[8].x = (ratio * (*Cam).ProjectioPlaneWidth) / 2;
	(*OBJ).WorldVertexList[8].y = -(ratio * (*Cam).ProjectionPlaneHeight) / 2;
	(*OBJ).WorldVertexList[8].z = ratio * (*Cam).ProjectionPlane_Z;


	ratio = (*Cam).FarPlane_Z / (*Cam).ProjectionPlane_Z;

	(*OBJ).WorldVertexList[9].x = -(ratio * (*Cam).ProjectioPlaneWidth) / 2;
	(*OBJ).WorldVertexList[9].y = -(ratio * (*Cam).ProjectionPlaneHeight) / 2;
	(*OBJ).WorldVertexList[9].z = ratio * (*Cam).ProjectionPlane_Z;

	(*OBJ).WorldVertexList[10].x = -(ratio * (*Cam).ProjectioPlaneWidth) / 2;
	(*OBJ).WorldVertexList[10].y = (ratio * (*Cam).ProjectionPlaneHeight) / 2;
	(*OBJ).WorldVertexList[10].z = ratio * (*Cam).ProjectionPlane_Z;

	(*OBJ).WorldVertexList[11].x = (ratio * (*Cam).ProjectioPlaneWidth) / 2;
	(*OBJ).WorldVertexList[11].y = (ratio * (*Cam).ProjectionPlaneHeight) / 2;
	(*OBJ).WorldVertexList[11].z = ratio * (*Cam).ProjectionPlane_Z;

	(*OBJ).WorldVertexList[12].x = (ratio * (*Cam).ProjectioPlaneWidth) / 2;
	(*OBJ).WorldVertexList[12].y = -(ratio * (*Cam).ProjectionPlaneHeight) / 2;
	(*OBJ).WorldVertexList[12].z = ratio * (*Cam).ProjectionPlane_Z;

	//Definicao dos triangulos

	//triangulos laterais
	(*OBJ).TriangleList[0] = 9;
	(*OBJ).TriangleList[1] = 0;
	(*OBJ).TriangleList[2] = 10;

	(*OBJ).TriangleList[3] = 10;
	(*OBJ).TriangleList[4] = 0;
	(*OBJ).TriangleList[5] = 11;

	(*OBJ).TriangleList[6] = 11;
	(*OBJ).TriangleList[7] = 0;
	(*OBJ).TriangleList[8] = 12;

	(*OBJ).TriangleList[9] = 12;
	(*OBJ).TriangleList[10] = 0;
	(*OBJ).TriangleList[11] = 9;

	//Triangulos do Plano de Projecao
	(*OBJ).TriangleList[12] = 1;
	(*OBJ).TriangleList[13] = 2;
	(*OBJ).TriangleList[14] = 3;

	(*OBJ).TriangleList[15] = 3;
	(*OBJ).TriangleList[16] = 4;
	(*OBJ).TriangleList[17] = 1;

	//Triangulos do Plano Proximo
	(*OBJ).TriangleList[18] = 5;
	(*OBJ).TriangleList[19] = 6;
	(*OBJ).TriangleList[20] = 7;

	(*OBJ).TriangleList[21] = 7;
	(*OBJ).TriangleList[22] = 8;
	(*OBJ).TriangleList[23] = 5;

	//Triangulos do Plano Distante
	(*OBJ).TriangleList[24] = 9;
	(*OBJ).TriangleList[25] = 10;
	(*OBJ).TriangleList[26] = 11;

	(*OBJ).TriangleList[27] = 11;
	(*OBJ).TriangleList[28] = 12;
	(*OBJ).TriangleList[29] = 9;


	for (i = 0; i < 30; i++)
	{
		(*OBJ).PSTriangleList[i] = (*OBJ).TriangleList[i];
	}



	for (i = 0; i < 13; i++)
	{
		Paux = (*OBJ).WorldVertexList[i];

		(*OBJ).WorldVertexList[i].x = (*Cam).camCS.base[0].x * Paux.x + (*Cam).camCS.base[1].x * Paux.y + (*Cam).camCS.base[2].x * Paux.z;
		(*OBJ).WorldVertexList[i].y = (*Cam).camCS.base[0].y * Paux.x + (*Cam).camCS.base[1].y * Paux.y + (*Cam).camCS.base[2].y * Paux.z;
		(*OBJ).WorldVertexList[i].z = (*Cam).camCS.base[0].z * Paux.x + (*Cam).camCS.base[1].z * Paux.y + (*Cam).camCS.base[2].z * Paux.z;

		(*OBJ).WorldVertexList[i].x += (*Cam).camCS.Origin.x;
		(*OBJ).WorldVertexList[i].y += (*Cam).camCS.Origin.y;
		(*OBJ).WorldVertexList[i].z += (*Cam).camCS.Origin.z;
	}

}

void drawLine(SDL_Renderer* renderer, int xi, int yi, int xf, int yf)
{
	int x1 = xi;
	int x2 = xf;
	int y1 = yi;
	int y2 = yf;
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;
	int err = dx - dy;

	while (1)
	{

		if (x1 <= 2500 && x1 >= 0 && y1 <= 1400 && y1 >= 0)
		{
			SDL_RenderDrawPoint(renderer, x1, y1);
			//SDL_RenderDrawPoint(renderer, x1, y1 + 1);
		}

		if (x1 == x2 && y1 == y2)
			break;

		int e2 = 2 * err;
		if (e2 > -dy)
		{
			err -= dy;
			x1 += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y1 += sy;
		}
	}
}
/*
void XYZ_SetSphericalObject(XYZPoint* S, int* V_index)
{
	int i, j;
	double Theta, Phi;

	i = 0;

	Theta = 0;
	Phi = pi / 10;

	while (Phi < pi)
	{
		while (Theta < 2 * pi)
		{
			S[i].x = cos(Theta) * sin(Phi);
			S[i].y = 1 + sin(Theta) * sin(Phi);
			S[i].z = cos(Phi);

			Theta += 2 * pi / 20;
			i++;
		}
		Theta = 0;
		Phi += pi / 10;
	}

	S[i].x = 0;
	S[i].y = 1;
	S[i].z = 1;

	i++;

	S[i].x = 0;
	S[i].y = 1;
	S[i].z = -1;

	j = 0;

	for (i = 0; i < 9; i ++)
	{
		while (j < (i + 1) * 20)
		{
			V_index[2 * j] = j;
			V_index[2 * j + 1] = j + 1;
			if(j + 1 == (i + 1) * 20) V_index[2 * j + 1] = j + 1 - 20;

			j ++;
		}
	}

	j = j * 2;

	for (i = 0; i < 20; i++)
	{
		V_index[j] = 180;
		j++;
		
		V_index[j] = 0 + i;
		j++;
		
		V_index[j] = 0 + i;
		j++;
		
		V_index[j] = 20 + i;
		j++;
		
		V_index[j] = 20 + i;
		j++;
		
		V_index[j] = 40 + i;
		j++;
		
		V_index[j] = 40 + i;
		j++;
		
		V_index[j] = 60 + i;
		j++;

		V_index[j] = 60 + i;
		j++;

		V_index[j] = 80 + i;
		j++;

		V_index[j] = 80 + i;
		j++;

		V_index[j] = 100 + i;
		j++;

		V_index[j] = 100 + i;
		j++;

		V_index[j] = 120 + i;
		j++;

		V_index[j] = 120 + i;
		j++;

		V_index[j] = 140 + i;
		j++;

		V_index[j] = 140 + i;
		j++;
		
		V_index[j] = 160 + i;
		j++;
		
		V_index[j] = 160 + i;
		j++;
		
		V_index[j] = 181;
		j++;


	}

	j = j;

}*/