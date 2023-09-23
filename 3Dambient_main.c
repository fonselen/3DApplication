#include <stdio.h>
#include <stdlib.h>
#include <SDL.h> // Para a biblioteca SDL
#include <math.h>

#define pi 3.14159265358979323
#define EXP 2.71828182845904523536

typedef struct
{
	double x; 		// define a coordenada x de um ponto no espaco
	double y;		// define a coordenada y de um ponto no espaco
	double z;		// define a coordenada z de um ponto no espaco
} XYZPoint;

typedef struct
{
	XYZPoint A; 	// ponto A de um segmento de reta no espaco
	XYZPoint B;		// ponto B de um segmento de reta no espaco
} XYZLine;

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
	double ZProjectionPlane;
	double ZFarPlane;
	double ProjectioPlaneWidth;
	double ProjectionPlaneHeight;
} XYZCam;

XYZPoint XYZ_PointRotation(XYZPoint P, XYZPoint O, double ang);
XYZPoint XYZ_PointTranslation(XYZPoint P, XYZPoint Vector, double t);
XYZTransformatioMatrix XYZ_SetRotationMatrix(XYZPoint RotationAxis, double ang);
XYZPoint XYZ_ApplyTransformationMatrix(XYZPoint P, XYZTransformatioMatrix M);
void Viewport_Render(XYZPoint* V, int* index, XYZCam Cam, int VNum);
void Viewport_Ground_Render(XYZPoint** V, XYZCam Cam, int NumVertices_i, int NumVertices_j);
void drawLine(SDL_Renderer* renderer, int xi, int yi, int xf, int yf);

void XYZ_SetSphericalObject(XYZPoint* S, int* V_index);


SDL_Renderer* renderer = NULL;

int main(int argc, char** argv)
{

	int i, j, k, MouseMH, MouseMV;
	int fat = 10;
	int GL, GC;
	XYZCam Cam1;
	XYZPoint CubeVertices[300][8], PiramideVertices[300][5], SphereVertices[1000];
	XYZPoint **GroundCoordinates;
	XYZTransformatioMatrix M, M_Esfera;
	int CubeIndex[300][24], PiramideIndex[300][16], SphereIndex[1000];
	XYZPoint P, WalkAxis;
	SDL_Event event;
	double RotCamAngAzimutal, RotCamAngPolar;
	XYZCoordinateSystem InertialCoordinateSystem;
	
	XYZ_SetSphericalObject(SphereVertices, SphereIndex);
	
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

	/*M_Esfera = XYZ_SetRotationMatrix(InertialCoordinateSystem.base[0], 90 * (pi / 180));

	for (i = 0; i < 182; i++)
	{
		SphereVertices[i] = XYZ_ApplyTransformationMatrix(SphereVertices[i], M_Esfera);
	}*/

	M_Esfera = XYZ_SetRotationMatrix(InertialCoordinateSystem.base[1], 1 * (pi / 180));


	GL = 5000;
	GC = 5000;

	// Alocação do ponteiro duplo
	GroundCoordinates = (XYZPoint**)malloc(GL * sizeof(XYZPoint*));
	
	if (GroundCoordinates == NULL)
	{
		printf("Erro ao alocar memória para o ponteiro duplo.\n");
		return 1;
	}

	// Alocação para cada linha do ponteiro duplo
	for (int i = 0; i < GL; i++)
	{
		
		GroundCoordinates[i] = (XYZPoint*)malloc(GC * sizeof(XYZPoint));
		
		if (GroundCoordinates[i] == NULL)
		{
			printf("Erro ao alocar memória para a linha %d.\n", i);
			return 1;
		}
	}

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
	Cam1.camCS.Origin.y = 0.5;
	Cam1.camCS.Origin.z = -10;

	Cam1.ZProjectionPlane = 0.5;

	WalkAxis = Cam1.camCS.base[2];

	P.x = 0;
	P.y = 0;
	P.z = 30;
	j = 0;
	for (i = 0; i < 300; i++)
	{
		if(j == 10)
		{
			//P.x += 2;
			//j = 0;
		}

		CubeVertices[i][0].x = P.x;
		CubeVertices[i][0].y = P.y;
		CubeVertices[i][0].z = P.z + 2 * j;
		CubeVertices[i][1].x = P.x + 1;
		CubeVertices[i][1].y = P.y;
		CubeVertices[i][1].z = P.z + 2 * j;
		CubeVertices[i][2].x = P.x + 1;
		CubeVertices[i][2].y = P.y + 1;
		CubeVertices[i][2].z = P.z + 2 * j;
		CubeVertices[i][3].x = P.x;
		CubeVertices[i][3].y = P.y + 1;
		CubeVertices[i][3].z = P.z + 2 * j;

		CubeVertices[i][4].x = P.x;
		CubeVertices[i][4].y = P.y;
		CubeVertices[i][4].z = P.z + 1 + 2 * j;
		CubeVertices[i][5].x = P.x + 1;
		CubeVertices[i][5].y = P.y;
		CubeVertices[i][5].z = P.z + 1 + 2 * j;
		CubeVertices[i][6].x = P.x + 1;
		CubeVertices[i][6].y = P.y + 1;
		CubeVertices[i][6].z = P.z + 1 + 2 * j;
		CubeVertices[i][7].x = P.x;
		CubeVertices[i][7].y = P.y + 1;
		CubeVertices[i][7].z = P.z + 1 + 2 * j;


		CubeIndex[i][0] = 0;
		CubeIndex[i][1] = 1;
		CubeIndex[i][2] = 1;
		CubeIndex[i][3] = 2;
		CubeIndex[i][4] = 2;
		CubeIndex[i][5] = 3;
		CubeIndex[i][6] = 3;
		CubeIndex[i][7] = 0;
		CubeIndex[i][8] = 4;
		CubeIndex[i][9] = 5;
		CubeIndex[i][10] = 5;
		CubeIndex[i][11] = 6;
		CubeIndex[i][12] = 6;
		CubeIndex[i][13] = 7;
		CubeIndex[i][14] = 7;
		CubeIndex[i][15] = 4;
		CubeIndex[i][16] = 0;
		CubeIndex[i][17] = 4;
		CubeIndex[i][18] = 1;
		CubeIndex[i][19] = 5;
		CubeIndex[i][20] = 2;
		CubeIndex[i][21] = 6;
		CubeIndex[i][22] = 3;
		CubeIndex[i][23] = 7;
		j++;

	}



	P.x = 0;
	P.y = 1.1;
	P.z = 30;
	j = 0;
	for (i = 0; i < 300; i++)
	{
		if (j == 10)
		{
			//P.x += 2;
			//j = 0;
		}

		PiramideVertices[i][0].x = P.x;
		PiramideVertices[i][0].y = P.y;
		PiramideVertices[i][0].z = P.z + 2 * j;
		PiramideVertices[i][1].x = P.x + 1;
		PiramideVertices[i][1].y = P.y;
		PiramideVertices[i][1].z = P.z + 2 * j;
		PiramideVertices[i][2].x = P.x + 1;
		PiramideVertices[i][2].y = P.y;
		PiramideVertices[i][2].z = P.z + 2 * j + 1;
		PiramideVertices[i][3].x = P.x;
		PiramideVertices[i][3].y = P.y;
		PiramideVertices[i][3].z = P.z + 2 * j + 1;
		PiramideVertices[i][4].x = P.x + (1.0 / 2.0);
		PiramideVertices[i][4].y = P.y +  1;
		PiramideVertices[i][4].z = P.z + 2 * j + (1.0 / 2.0);

		PiramideIndex[i][0] = 0;
		PiramideIndex[i][1] = 1;
		PiramideIndex[i][2] = 1;
		PiramideIndex[i][3] = 2;
		PiramideIndex[i][4] = 2;
		PiramideIndex[i][5] = 3;
		PiramideIndex[i][6] = 3;
		PiramideIndex[i][7] = 0;
		PiramideIndex[i][8] = 0;
		PiramideIndex[i][9] = 4;
		PiramideIndex[i][10] = 1;
		PiramideIndex[i][11] = 4;
		PiramideIndex[i][12] = 2;
		PiramideIndex[i][13] = 4;
		PiramideIndex[i][14] = 3;
		PiramideIndex[i][15] = 4;
		
		
		j++;

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
				printf("\n Azimutal = %lf - Polar = %lf", RotCamAngAzimutal, RotCamAngPolar);

				M = XYZ_SetRotationMatrix(InertialCoordinateSystem.base[1], RotCamAngAzimutal);

				Cam1.camCS.base[0] = XYZ_ApplyTransformationMatrix(Cam1.camCS.base[0], M);
				Cam1.camCS.base[1] = XYZ_ApplyTransformationMatrix(Cam1.camCS.base[1], M);
				Cam1.camCS.base[2] = XYZ_ApplyTransformationMatrix(Cam1.camCS.base[2], M);
				WalkAxis = XYZ_ApplyTransformationMatrix(WalkAxis, M);

				M = XYZ_SetRotationMatrix(Cam1.camCS.base[0], RotCamAngPolar);

				Cam1.camCS.base[1] = XYZ_ApplyTransformationMatrix(Cam1.camCS.base[1], M);
				Cam1.camCS.base[2] = XYZ_ApplyTransformationMatrix(Cam1.camCS.base[2], M);

				//Cam1.camCS.base[0] = XYZ_PointRotation(Cam1.camCS.base[0], Cam1.camCS.base[0], RotCamAngAzimutal);
				//Cam1.camCS.base[2] = XYZ_PointRotation(Cam1.camCS.base[2], Cam1.camCS.base[2], RotCamAngAzimutal);


			}

			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE) k = 1;

				if (event.key.keysym.sym == SDLK_w)
				{
					Cam1.camCS.Origin = XYZ_PointTranslation(Cam1.camCS.Origin, WalkAxis, 0.2);

				}

				if (event.key.keysym.sym == SDLK_s)
				{
					Cam1.camCS.Origin = XYZ_PointTranslation(Cam1.camCS.Origin, WalkAxis, -0.2);

				}

				if (event.key.keysym.sym == SDLK_d)
				{
					Cam1.camCS.Origin = XYZ_PointTranslation(Cam1.camCS.Origin, Cam1.camCS.base[0], 0.1);

				}

				if (event.key.keysym.sym == SDLK_a)
				{
					Cam1.camCS.Origin = XYZ_PointTranslation(Cam1.camCS.Origin, Cam1.camCS.base[0], -0.1);

				}

			}
		}
	
		

		Viewport_Ground_Render(GroundCoordinates, Cam1, GL, GC);


		SDL_SetRenderDrawColor(renderer, 50, 50, 250, 255);
		/*for (i = 0; i < 300; i++)
		{
			Viewport_Render(CubeVertices[i], CubeIndex[i], Cam1, 12);
		}*/

		SDL_SetRenderDrawColor(renderer, 50, 50, 250, 255);
		/*for (i = 0; i < 300; i++)
		{
			Viewport_Render(PiramideVertices[i], PiramideIndex[i], Cam1, 8);
		}*/
		
		Viewport_Render(SphereVertices, SphereIndex, Cam1, 380);

		/* or (i = 0; i < 300; i++)
		{
			for (j = 0; j < 8; j++)
			{
				//CubeVertices[i][j].z = CubeVertices[i][j].z - 0.05;
			}
			
		}*/

		/*for (i = 0; i < 300; i++)
		{
			for (j = 0; j < 5; j++)
			{
				PiramideVertices[i][j].z = PiramideVertices[i][j].z - 0.05;
				PiramideVertices[i][j].y = PiramideVertices[i][j].y + 0.001;
			}

		}*/

		for (i = 0; i < 182; i++)
		{
			SphereVertices[i] = XYZ_ApplyTransformationMatrix(SphereVertices[i], M_Esfera);
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


void Viewport_Render(XYZPoint* V, int* index, XYZCam Cam, int VNum)
{

	XYZPoint Pa, Pb, Paux, Dir;
	int i;
	double d = 0.5, t;
	
	for (i = 0; i < VNum; i++)
	{
		Pa = V[ index[2 * i] ];
		Pb = V[index[2 * i + 1]];

		Paux = Pa;
		Paux.x = Paux.x - Cam.camCS.Origin.x;
		Paux.y = Paux.y - Cam.camCS.Origin.y;
		Paux.z = Paux.z - Cam.camCS.Origin.z;
	
		Pa.x = Cam.camCS.base[0].x * Paux.x + Cam.camCS.base[0].y * Paux.y + Cam.camCS.base[0].z * Paux.z;
		Pa.y = Cam.camCS.base[1].x * Paux.x + Cam.camCS.base[1].y * Paux.y + Cam.camCS.base[1].z * Paux.z;
		Pa.z = Cam.camCS.base[2].x * Paux.x + Cam.camCS.base[2].y * Paux.y + Cam.camCS.base[2].z * Paux.z;

		Paux = Pb;
		Paux.x = Paux.x - Cam.camCS.Origin.x;
		Paux.y = Paux.y - Cam.camCS.Origin.y;
		Paux.z = Paux.z - Cam.camCS.Origin.z;

		Pb.x = Cam.camCS.base[0].x * Paux.x + Cam.camCS.base[0].y * Paux.y + Cam.camCS.base[0].z * Paux.z;
		Pb.y = Cam.camCS.base[1].x * Paux.x + Cam.camCS.base[1].y * Paux.y + Cam.camCS.base[1].z * Paux.z;
		Pb.z = Cam.camCS.base[2].x * Paux.x + Cam.camCS.base[2].y * Paux.y + Cam.camCS.base[2].z * Paux.z;


		/*if (Pa.z > 100 && Pb.z > 00) return;
		if (Pa.x > 50 && Pb.x > 50) return;
		if (Pa.x < -50 && Pb.x < -50) return;*/

		if ((Pa.x <= 25 || Pb.x <= 25) && (Pa.x >= -25 || Pb.x >= -25) && (Pa.z <= 50 || Pb.z <= 50))
		{
			if (Pa.z >= Cam.ZProjectionPlane || Pb.z >= Cam.ZProjectionPlane)
			{
				
				if (Pa.z < Cam.ZProjectionPlane)
				{
					Dir.x = Pa.x - Pb.x;
					Dir.y = Pa.y - Pb.y;
					Dir.z = Pa.z - Pb.z;

					t = (Cam.ZProjectionPlane - Pb.z) / Dir.z;

					Pa.x = Pb.x + t * Dir.x;
					Pa.y = Pb.y + t * Dir.y;
					Pa.z = Cam.ZProjectionPlane;

				}

				if (Pb.z < Cam.ZProjectionPlane)
				{
					Dir.x = Pb.x - Pa.x;
					Dir.y = Pb.y - Pa.y;
					Dir.z = Pb.z - Pa.z;


					t = (Cam.ZProjectionPlane - Pa.z) / Dir.z;

					Pb.x = Pa.x + t * Dir.x;
					Pb.y = Pa.y + t * Dir.y;
					Pb.z = Cam.ZProjectionPlane;
				}

				Pa.x = (d / (Pa.z)) * Pa.x;
				Pa.y = (d / (Pa.z)) * Pa.y;
				Pa.z = Cam.ZProjectionPlane;

				Pb.x = (d / (Pb.z)) * Pb.x;
				Pb.y = (d / (Pb.z)) * Pb.y;
				Pa.z = Cam.ZProjectionPlane;

				Pa.x = (2500 / d) * Pa.x + 1250;
				Pa.y = (-2500 / d) * Pa.y + 700;
				Pb.x = (2500 / d) * Pb.x + 1250;
				Pb.y = (-2500 / d) * Pb.y + 700;

				drawLine(renderer, Pa.x, Pa.y, Pb.x, Pb.y);
				//SDL_RenderPresent(renderer);
			}

		//end if
		}
		
	//end for
	}


}

void Viewport_Ground_Render(XYZPoint** V, XYZCam Cam, int NumVertices_i, int NumVertices_j)
{
	XYZPoint Pa, Pb, Paux, Dir;
	int i, j, VMax_i, VMax_j;
	double d = 0.5, t, xmin, xMax, zmin, zMax;

	// Aqui os pontos Pa, Pb e Paux serao usados como pontos auxiliares no calculo da posicao na matriz de vertices
	Paux = Cam.camCS.Origin; // origem da camera
	
	//Ponto extremo a esquerda
	Pa.x = Cam.camCS.base[0].x * (-25) + Cam.camCS.base[1].x * 0 + Cam.camCS.base[2].x * 50 + Paux.x;
	Pa.y = 0;
	Pa.z = Cam.camCS.base[0].z * (-25) + Cam.camCS.base[1].z * 0 + Cam.camCS.base[2].z * 50 + Paux.z;
	
	//Ponto extremo a direita
	Pb.x = Cam.camCS.base[0].x * (25) + Cam.camCS.base[1].x * 0 + Cam.camCS.base[2].x * 50 + Paux.x;
	Pb.y = 0;
	Pb.z = Cam.camCS.base[0].z * (25) + Cam.camCS.base[1].z * 0 + Cam.camCS.base[2].z * 50 + Paux.z;
	
	xmin = Paux.x;
	xMax = Paux.x;
	zmin = Paux.z;
	zMax = Paux.z;


	if (Pa.x < xmin) xmin = Pa.x;
	if (Pb.x < xmin) xmin = Pb.x;

	if (Pa.x > xMax) xMax = Pa.x;
	if (Pb.x > xMax) xMax = Pb.x;

	if (Pa.z < zmin) zmin = Pa.z;
	if (Pb.z < zmin) zmin = Pb.z;

	if (Pa.z > zMax) zMax = Pa.z;
	if (Pb.z > zMax) zMax = Pb.z;

	i = (int)round(zmin);
	j = (int)round(xmin);

	i = i - 1 + (NumVertices_i) / 2;
	j = j - 1 + (NumVertices_j) / 2;

	if (i < 0) i = 0;
	if (j < 0) j = 0;

	VMax_i = (int)round(zMax);
	VMax_j = (int)round(xMax);

	VMax_i = VMax_i + (NumVertices_i) / 2;
	VMax_j = VMax_j + (NumVertices_j) / 2;

	while(i < VMax_i)
	{
		while(j < VMax_j)
		{
			Pa = V[i][j];
			Pb = V[i][j + 1];

			Paux = Pa;
			Paux.x = Paux.x - Cam.camCS.Origin.x;
			Paux.y = Paux.y - Cam.camCS.Origin.y;
			Paux.z = Paux.z - Cam.camCS.Origin.z;

			Pa.x = Cam.camCS.base[0].x * Paux.x + Cam.camCS.base[0].y * Paux.y + Cam.camCS.base[0].z * Paux.z;
			Pa.y = Cam.camCS.base[1].x * Paux.x + Cam.camCS.base[1].y * Paux.y + Cam.camCS.base[1].z * Paux.z;
			Pa.z = Cam.camCS.base[2].x * Paux.x + Cam.camCS.base[2].y * Paux.y + Cam.camCS.base[2].z * Paux.z;

			Paux = Pb;
			Paux.x = Paux.x - Cam.camCS.Origin.x;
			Paux.y = Paux.y - Cam.camCS.Origin.y;
			Paux.z = Paux.z - Cam.camCS.Origin.z;

			Pb.x = Cam.camCS.base[0].x * Paux.x + Cam.camCS.base[0].y * Paux.y + Cam.camCS.base[0].z * Paux.z;
			Pb.y = Cam.camCS.base[1].x * Paux.x + Cam.camCS.base[1].y * Paux.y + Cam.camCS.base[1].z * Paux.z;
			Pb.z = Cam.camCS.base[2].x * Paux.x + Cam.camCS.base[2].y * Paux.y + Cam.camCS.base[2].z * Paux.z;




			if ((Pa.x <= 25 || Pb.x <= 25) && (Pa.x >= -25 || Pb.x >= -25) && (Pa.z <= 50 || Pb.z <= 50))
			{
				if (Pa.z >= Cam.ZProjectionPlane || Pb.z >= Cam.ZProjectionPlane)
				{
					SDL_SetRenderDrawColor(renderer, 255 - 5 * ((int)round(Pa.z)), 255 - 5 * ((int)round(Pa.z)), 255 - 5 * ((int)round(Pa.z)), 255);
					if (Pa.z < Cam.ZProjectionPlane)
					{
						Dir.x = Pa.x - Pb.x;
						Dir.y = Pa.y - Pb.y;
						Dir.z = Pa.z - Pb.z;

						t = (Cam.ZProjectionPlane - Pb.z) / Dir.z;

						Pa.x = Pb.x + t * Dir.x;
						Pa.y = Pb.y + t * Dir.y;
						Pa.z = Cam.ZProjectionPlane;

					}

					if (Pb.z < Cam.ZProjectionPlane)
					{
						Dir.x = Pb.x - Pa.x;
						Dir.y = Pb.y - Pa.y;
						Dir.z = Pb.z - Pa.z;


						t = (Cam.ZProjectionPlane - Pa.z) / Dir.z;

						Pb.x = Pa.x + t * Dir.x;
						Pb.y = Pa.y + t * Dir.y;
						Pb.z = Cam.ZProjectionPlane;
					}

					Pa.x = (d / (Pa.z)) * Pa.x;
					Pa.y = (d / (Pa.z)) * Pa.y;
					Pa.z = Cam.ZProjectionPlane;

					Pb.x = (d / (Pb.z)) * Pb.x;
					Pb.y = (d / (Pb.z)) * Pb.y;
					Pa.z = Cam.ZProjectionPlane;

					Pa.x = (2500 / d) * Pa.x + 1250;
					Pa.y = (-2500 / d) * Pa.y + 700;
					Pb.x = (2500 / d) * Pb.x + 1250;
					Pb.y = (-2500 / d) * Pb.y + 700;

					drawLine(renderer, Pa.x, Pa.y, Pb.x, Pb.y);
				}

			}

			j++;
		}
		j = (int)round(xmin) - 1 + (NumVertices_j) / 2;
		i++;
	}



}


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

}