#include <stdio.h>
#include <stdlib.h>
#include <SDL.h> // Para a biblioteca SDL
#include <conio.h>
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
	XYZCoordinateSystem camCS;
	double ZPlane;
} XYZCam;

XYZPoint XYZ_PointRotation(XYZPoint P, XYZPoint O, double ang);
XYZPoint XYZ_PointTranslation(XYZPoint P, XYZPoint Vector, double t);
void Viewport_Render(XYZPoint* V, int* index, XYZCam Cam, int VNum);
void Viewport_Ground_Render(XYZPoint** V, XYZCam Cam, int NumVertices_i, int NumVertices_j);
void drawLine(SDL_Renderer* renderer, int xi, int yi, int xf, int yf);

SDL_Renderer* renderer = NULL;

int main(int argc, char** argv) {

	int i, j, k, MouseM;
	int fat = 10;
	int GL, GC;
	XYZCam Cam1;
	XYZPoint CubeVertices[10][8];
	XYZPoint **GroundCoordinates;
	int CubeIndex[10][24], **GroundIndex1;
	XYZPoint P;
	SDL_Event event;
	double RotCamAng;

	
	GL = 10000;
	GC = 10000;

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
			GroundCoordinates[i][j].y = 0;
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

	Cam1.ZPlane = 0.5;

	

	P.x = 0;
	P.y = 0;
	P.z = 30;
	

	for (i = 0; i < 10; i++)
	{
		CubeVertices[i][0].x = P.x;
		CubeVertices[i][0].y = P.y;
		CubeVertices[i][0].z = P.z + 2 * i;
		CubeVertices[i][1].x = P.x + 1;
		CubeVertices[i][1].y = P.y;
		CubeVertices[i][1].z = P.z + 2 * i;
		CubeVertices[i][2].x = P.x + 1;
		CubeVertices[i][2].y = P.y + 1;
		CubeVertices[i][2].z = P.z + 2 * i;
		CubeVertices[i][3].x = P.x;
		CubeVertices[i][3].y = P.y + 1;
		CubeVertices[i][3].z = P.z + 2 * i;

		CubeVertices[i][4].x = P.x;
		CubeVertices[i][4].y = P.y;
		CubeVertices[i][4].z = P.z + 1 + 2 * i;
		CubeVertices[i][5].x = P.x + 1;
		CubeVertices[i][5].y = P.y;
		CubeVertices[i][5].z = P.z + 1 + 2 * i;
		CubeVertices[i][6].x = P.x + 1;
		CubeVertices[i][6].y = P.y + 1;
		CubeVertices[i][6].z = P.z + 1 + 2 * i;
		CubeVertices[i][7].x = P.x;
		CubeVertices[i][7].y = P.y + 1;
		CubeVertices[i][7].z = P.z + 1 + 2 * i;


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
				MouseM = event.motion.xrel;

				RotCamAng = (-MouseM * pi / 180) / 20;

				Cam1.camCS.base[0] = XYZ_PointRotation(Cam1.camCS.base[0], Cam1.camCS.base[0], RotCamAng);
				Cam1.camCS.base[2] = XYZ_PointRotation(Cam1.camCS.base[2], Cam1.camCS.base[2], RotCamAng);


			}

			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE) k = 1;

				if (event.key.keysym.sym == SDLK_w)
				{
					Cam1.camCS.Origin = XYZ_PointTranslation(Cam1.camCS.Origin, Cam1.camCS.base[2], 0.2);

				}

				if (event.key.keysym.sym == SDLK_s)
				{
					Cam1.camCS.Origin = XYZ_PointTranslation(Cam1.camCS.Origin, Cam1.camCS.base[2], -0.2);

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
		for (i = 0; i < 10; i++)
		{
			Viewport_Render(CubeVertices[i], CubeIndex[i], Cam1, 12);
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
			if (Pa.z >= Cam.ZPlane || Pb.z >= Cam.ZPlane)
			{
				
				if (Pa.z < Cam.ZPlane)
				{
					Dir.x = Pa.x - Pb.x;
					Dir.y = Pa.y - Pb.y;
					Dir.z = Pa.z - Pb.z;

					t = (Cam.ZPlane - Pb.z) / Dir.z;

					Pa.x = Pb.x + t * Dir.x;
					Pa.y = Pb.y + t * Dir.y;
					Pa.z = Cam.ZPlane;

				}

				if (Pb.z < Cam.ZPlane)
				{
					Dir.x = Pb.x - Pa.x;
					Dir.y = Pb.y - Pa.y;
					Dir.z = Pb.z - Pa.z;


					t = (Cam.ZPlane - Pa.z) / Dir.z;

					Pb.x = Pa.x + t * Dir.x;
					Pb.y = Pa.y + t * Dir.y;
					Pb.z = Cam.ZPlane;
				}

				Pa.x = (d / (Pa.z)) * Pa.x;
				Pa.y = (d / (Pa.z)) * Pa.y;
				Pa.z = Cam.ZPlane;

				Pb.x = (d / (Pb.z)) * Pb.x;
				Pb.y = (d / (Pb.z)) * Pb.y;
				Pa.z = Cam.ZPlane;

				Pa.x = (2500 / d) * Pa.x + 1250;
				Pa.y = (-2500 / d) * Pa.y + 700;
				Pb.x = (2500 / d) * Pb.x + 1250;
				Pb.y = (-2500 / d) * Pb.y + 700;

				drawLine(renderer, Pa.x, Pa.y, Pb.x, Pb.y);
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
				if (Pa.z >= Cam.ZPlane || Pb.z >= Cam.ZPlane)
				{
					SDL_SetRenderDrawColor(renderer, 255 - 5 * ((int)round(Pa.z)), 255 - 5 * ((int)round(Pa.z)), 255 - 5 * ((int)round(Pa.z)), 255);
					if (Pa.z < Cam.ZPlane)
					{
						Dir.x = Pa.x - Pb.x;
						Dir.y = Pa.y - Pb.y;
						Dir.z = Pa.z - Pb.z;

						t = (Cam.ZPlane - Pb.z) / Dir.z;

						Pa.x = Pb.x + t * Dir.x;
						Pa.y = Pb.y + t * Dir.y;
						Pa.z = Cam.ZPlane;

					}

					if (Pb.z < Cam.ZPlane)
					{
						Dir.x = Pb.x - Pa.x;
						Dir.y = Pb.y - Pa.y;
						Dir.z = Pb.z - Pa.z;


						t = (Cam.ZPlane - Pa.z) / Dir.z;

						Pb.x = Pa.x + t * Dir.x;
						Pb.y = Pa.y + t * Dir.y;
						Pb.z = Cam.ZPlane;
					}

					Pa.x = (d / (Pa.z)) * Pa.x;
					Pa.y = (d / (Pa.z)) * Pa.y;
					Pa.z = Cam.ZPlane;

					Pb.x = (d / (Pb.z)) * Pb.x;
					Pb.y = (d / (Pb.z)) * Pb.y;
					Pa.z = Cam.ZPlane;

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