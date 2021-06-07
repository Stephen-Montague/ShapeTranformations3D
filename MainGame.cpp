#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Frog.h"
#include "MainGame.h"
#include "MainUpdate.h"

using namespace Webfoot;

MainGame MainGame::instance;

//==============================================================================

#define GUI_LAYER_NAME "MainGame"

#define DEFAULT_INPUT 'N'  // No user input

#define ROTATION_RATE 4.0f  // Degree(s) per update cycle
#define SCALAR 1.125f // To scale sprite in or out
#define TRANSLATION 8.0f // Distance per update cycle
#define WORLD_SIZE 10000.0f // Size of a cubic world

//-----------------------------------------------------------------------------

MainGame::MainGame()
{
   sprite = NULL;
}

//-----------------------------------------------------------------------------

void MainGame::Init()
{
   Inherited::Init();

   // Initialize sprite
   sprite = frog_new Sprite_X();
   sprite->Init();

   // Default game input
   userInput = DEFAULT_INPUT;
}

//-----------------------------------------------------------------------------

void MainGame::Deinit()
{
   // Deinitialize and delete sprite
	if (sprite)
	{
		sprite->Deinit();
		frog_delete sprite;
		sprite = NULL;
	}
   Inherited::Deinit();
}

//-----------------------------------------------------------------------------

const char* MainGame::GUILayerNameGet()
{
   return GUI_LAYER_NAME;
}

//-----------------------------------------------------------------------------

void MainGame::Update()
{
   Inherited::Update();

   if (!theMainUpdate->ExitingCheck() && theKeyboard->KeyJustPressed(KEY_Q) || theKeyboard->KeyJustPressed(KEY_ESCAPE))
   {
	   theMainGame->StateChangeTransitionBegin(true);
	   theStates->Pop();
	   theMainUpdate->Exit();
   }   

   if (!theStates->StateChangeCheck() && theKeyboard->KeyPressed() || theKeyboard->KeyJustPressed())
   {
	   for (int i = 0; i < theKeyboard->TextInputCountGet(); i++)
	   {
		   const char* keyInput = theKeyboard->TextInputGet(i);
		   userInput = *keyInput;
		   DebugPrintf("Key pressed: %c\n", userInput);

		   sprite->Animate(userInput);
	   }
   }
}

//-----------------------------------------------------------------------------

void MainGame::Draw()
{	
	sprite->Draw();
}

//-----------------------------------------------------------------------------


//==============================================================================

Sprite_X::Sprite_X()
{
	// Initialize each Table matrix.
	M_shape.Init();
	M_reset.Init();
	M_draw.Init();
}

void Sprite_X::Init()
{
	// Set sprite shape matrix.
	M_shape = GetShapeVertices();

	// Set reset matrix.
	M_reset = M_shape;
}

void Sprite_X::Deinit()
{
	// Deinit tables
	M_shape.Deinit();
	M_reset.Deinit();
	M_draw.Deinit();
}

void Sprite_X::Animate(char action)
{
	using namespace Transformation;

	switch (action)
	{
	case 'l':  // Move left.
	case 'L':
		Translate(M_shape, -TRANSLATION, 0.0f, 0.0f);
		break;
	case 'r':  // Move right.
	case 'R':
		Translate(M_shape, TRANSLATION, 0.0f, 0.0f);
		break;
	case 'u':  // Move up.
	case 'U':
		Translate(M_shape, 0.0f, -TRANSLATION, 0.0f);
		break;
	case 'd':  // Move down.
	case 'D':
		Translate(M_shape, 0.0f, TRANSLATION, 0.0f);
		break;
	case 'i':  // Zoom in.
	case 'I':
		Scale(M_shape, 1.0f / SCALAR);
		break;
	case 'o':  // Zoom out.
	case 'O':
		Scale(M_shape, SCALAR);
		break;
	case 'x':  // Rotate on X axis
	case 'X':
		RotateX(M_shape, ROTATION_RATE);
		break;
	case 'y':  // Rotate on Y axis
	case 'Y':
		RotateY(M_shape, ROTATION_RATE);
		break;
	case 'z':  // Rotate on Z axis
	case 'Z':
		RotateZ(M_shape, ROTATION_RATE);
		break;
	case '1':  // Reset shape to original condition
		M_shape = M_reset;
		break;
	default:  // No transformation.
		break;
	}
}

void Sprite_X::Draw()
{
	// Draw pyramid
	M_draw = M_shape;
	M_draw.RemoveBack();  // Remove homogenous coordinate.
	for (int i = 0; i < M_draw.SizeGet() - 1; i++)
	{
		if (M_draw[i] == jump || M_draw[i + 1] == jump)
		{
			continue;
		}
		else
		{
			// Convert from Point4F since LineDraw() takes only 2d or 3d points.
			Point2F point1 = Point2F::Create(M_draw[i].x, M_draw[i].y);  
			Point2F point2 = Point2F::Create(M_draw[i+1].x, M_draw[i+1].y);
			
			// Draw each line.
			theScreen->LineDraw(point1, point2, COLOR_RGBA8_GREEN, 3.0f, 0.0f);
		}
	}
}

Table<Point4F> Sprite_X::GetShapeVertices()
{
	using namespace std;
	ifstream inFile;
	string line;

	float worldWidth = WORLD_SIZE;  
	float worldHeight = WORLD_SIZE; 
	float worldDepth = WORLD_SIZE;
	float screenWidth = float(theScreen->WidthGet());
	float screenHeight = float(theScreen->HeightGet());
	float screenDepth = float(theScreen->WidthGet());  // Use screen width for depth conversion.

	jump = Point4F::Create(-1.0f, -1.0f, -1.0f, -1.0f);

	// Open file & convert each line to screen coordinates 
	inFile.open("/PIXA.DAT");
	if (inFile.is_open())
	{
		for (int row = 0; getline(inFile, line); row++)
		{
			if (line.at(0) == 'J')
			{
				M_shape.AddBack(jump);
			}
			else
			{
				istringstream iss(line);
				float x, y, z;
				iss >> x >> y >> z;
				coordinate = Point4F::Create(x, y, z, 1.0f);  // w = 1.

				// Convert world range to base 0
				coordinate.x += worldWidth / 2.0f;
				coordinate.y += worldHeight / 2.0f;
				coordinate.z += worldDepth / 2.0f;

				// Convert world to screen ratio
				coordinate.x *= (screenWidth / worldWidth);
				coordinate.y *= (screenHeight / worldHeight);
				coordinate.z *= (screenDepth / worldDepth);

				M_shape.AddBack(coordinate);
			}
		}
	}
	else
	{
		DebugPrintf("\nError:");
		DebugPrintf("\nCould not open file to read data.\nPlease ensure sprite data file is in project directory.\n");
	}

	M_shape.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));  // Add homogenous coordinate

	return M_shape;
}

namespace Webfoot {
	namespace Transformation {

		void Translate(Table <Point4F> &shape, float deltaX, float deltaY, float deltaZ)
		{
			Table <Point4F> M_result;
			Table <Point4F> translation;

			M_result.Init(shape);
			translation.Init();

			Point4F jump = Point4F::Create(-1.0f, -1.0f, -1.0f, -1.0f);

			// Build matrix: translate to origin
			translation.AddBack(Point4F::Create(1.0f, 0.0f, 0.0f, deltaX));
			translation.AddBack(Point4F::Create(0.0f, 1.0f, 0.0f, deltaY));
			translation.AddBack(Point4F::Create(0.0f, 0.0f, 1.0f, deltaZ));
			translation.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Translate shape
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(translation[j]);
					}
				}
			}
			shape = M_result;

			translation.Deinit();
			M_result.Deinit();
		}

		void Scale(Table <Point4F> &shape, float scalar)
		{
			Table <Point4F> M_result;
			Table <Point4F> transOrigin;
			Table <Point4F> scale;
			Table <Point4F> transBack;

			M_result.Init(shape);
			transOrigin.Init();
			scale.Init();
			transBack.Init();

			Point4F jump = Point4F::Create(-1.0f, -1.0f, -1.0f, -1.0f);

			// Build matrix: translate to origin
			float deltaX = float(-1.0f * shape[1].x);
			float deltaY = float(-1.0f * shape[1].y);
			float deltaZ = float(-1.0f * shape[1].z);
			transOrigin.AddBack(Point4F::Create(1.0f, 0.0f, 0.0f, deltaX));
			transOrigin.AddBack(Point4F::Create(0.0f, 1.0f, 0.0f, deltaY));
			transOrigin.AddBack(Point4F::Create(0.0f, 0.0f, 1.0f, deltaZ));
			transOrigin.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Build matrix: scale 
			scale.AddBack(Point4F::Create(scalar, 0.0f, 0.0f, 0.0f));
			scale.AddBack(Point4F::Create(0.0f, scalar, 0.0f, 0.0f));
			scale.AddBack(Point4F::Create(0.0f, 0.0f, scalar, 0.0f));
			scale.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Build matrix: translate back 	
			deltaX *= -1.0f;
			deltaY *= -1.0f;
			deltaZ *= -1.0f;
			transBack.AddBack(Point4F::Create(1.0f, 0.0f, 0.0f, deltaX));
			transBack.AddBack(Point4F::Create(0.0f, 1.0f, 0.0f, deltaY));
			transBack.AddBack(Point4F::Create(0.0f, 0.0f, 1.0f, deltaZ));
			transBack.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Translate to origin
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(transOrigin[j]);
					}
				}
			}
			shape = M_result;

			// Scale shape
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(scale[j]);
					}
				}
			}
			shape = M_result;

			// Translate back
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(transBack[j]);
					}
				}
			}
			shape = M_result;

			M_result.Deinit();
			transOrigin.Deinit();
			scale.Deinit();
			transBack.Deinit();
		}

		void RotateX(Table <Point4F> &shape, float rotation)
		{
			Table <Point4F> M_result;
			Table <Point4F> transOrigin;
			Table <Point4F> rotateX;
			Table <Point4F> transBack;

			M_result.Init(shape);
			transOrigin.Init();
			rotateX.Init();
			transBack.Init();

			Point4F jump = Point4F::Create(-1.0f, -1.0f, -1.0f, -1.0f);

			// Build matrix: translate to origin
			float deltaX = float(-1.0f * shape[1].x);
			float deltaY = float(-1.0f * shape[1].y);
			float deltaZ = float(-1.0f * shape[1].z);
			transOrigin.AddBack(Point4F::Create(1.0f, 0.0f, 0.0f, deltaX));
			transOrigin.AddBack(Point4F::Create(0.0f, 1.0f, 0.0f, deltaY));
			transOrigin.AddBack(Point4F::Create(0.0f, 0.0f, 1.0f, deltaZ));
			transOrigin.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Build matrix: rotate on X axis
			rotateX.AddBack(Point4F::Create(1.0f, 0.0f, 0.0f, 0.0f));
			rotateX.AddBack(Point4F::Create(0.0f, FrogMath::Cos(ROTATION_RATE), FrogMath::Sin(-ROTATION_RATE), 0.0f));
			rotateX.AddBack(Point4F::Create(0.0f, FrogMath::Sin(ROTATION_RATE), FrogMath::Cos(ROTATION_RATE), 0.0f));
			rotateX.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Build matrix: translate back 	
			deltaX *= -1.0f;
			deltaY *= -1.0f;
			deltaZ *= -1.0f;
			transBack.AddBack(Point4F::Create(1.0f, 0.0f, 0.0f, deltaX));
			transBack.AddBack(Point4F::Create(0.0f, 1.0f, 0.0f, deltaY));
			transBack.AddBack(Point4F::Create(0.0f, 0.0f, 1.0f, deltaZ));
			transBack.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Translate to origin
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(transOrigin[j]);
					}
				}
			}
			shape = M_result;

			// Rotate
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(rotateX[j]);
					}
				}
			}
			shape = M_result;

			// Translate back
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(transBack[j]);
					}
				}
			}
			shape = M_result;

			M_result.Deinit();
			transOrigin.Deinit();
			rotateX.Deinit();
			transBack.Deinit();
		}

		void RotateY(Table <Point4F> &shape, float rotation)
		{
			Table <Point4F> M_result;
			Table <Point4F> transOrigin;
			Table <Point4F> rotateY;
			Table <Point4F> transBack;

			M_result.Init(shape);
			transOrigin.Init();
			rotateY.Init();
			transBack.Init();

			Point4F jump = Point4F::Create(-1.0f, -1.0f, -1.0f, -1.0f);

			// Build matrix: translate to origin
			float deltaX = float(-1.0f * shape[1].x);
			float deltaY = float(-1.0f * shape[1].y);
			float deltaZ = float(-1.0f * shape[1].z);
			transOrigin.AddBack(Point4F::Create(1.0f, 0.0f, 0.0f, deltaX));
			transOrigin.AddBack(Point4F::Create(0.0f, 1.0f, 0.0f, deltaY));
			transOrigin.AddBack(Point4F::Create(0.0f, 0.0f, 1.0f, deltaZ));
			transOrigin.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Build matrix: rotate on Z axis
			rotateY.AddBack(Point4F::Create(FrogMath::Cos(ROTATION_RATE), 0.0f, FrogMath::Sin(ROTATION_RATE), 0.0f));
			rotateY.AddBack(Point4F::Create(0.0f, 1.0f, 0.0f, 0.0f));
			rotateY.AddBack(Point4F::Create(FrogMath::Sin(-ROTATION_RATE), 0.0f, FrogMath::Cos(ROTATION_RATE), 0.0f));
			rotateY.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Build matrix: translate back 	
			deltaX *= -1.0f;
			deltaY *= -1.0f;
			deltaZ *= -1.0f;
			transBack.AddBack(Point4F::Create(1.0f, 0.0f, 0.0f, deltaX));
			transBack.AddBack(Point4F::Create(0.0f, 1.0f, 0.0f, deltaY));
			transBack.AddBack(Point4F::Create(0.0f, 0.0f, 1.0f, deltaZ));
			transBack.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Translate to origin
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(transOrigin[j]);
					}
				}
			}
			shape = M_result;

			// Rotate
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(rotateY[j]);
					}
				}
			}
			shape = M_result;

			// Translate back
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(transBack[j]);
					}
				}
			}
			shape = M_result;

			M_result.Deinit();
			transOrigin.Deinit();
			rotateY.Deinit();
			transBack.Deinit();

		}

		void RotateZ(Table <Point4F> &shape, float rotation)
		{
			Table <Point4F> M_result;
			Table <Point4F> transOrigin;
			Table <Point4F> rotateZ;
			Table <Point4F> transBack;

			M_result.Init(shape);
			transOrigin.Init();
			rotateZ.Init();
			transBack.Init();

			Point4F jump = Point4F::Create(-1.0f, -1.0f, -1.0f, -1.0f);

			// Build matrix: translate to origin
			float deltaX = float(-1.0f * shape[1].x);
			float deltaY = float(-1.0f * shape[1].y);
			float deltaZ = float(-1.0f * shape[1].z);
			transOrigin.AddBack(Point4F::Create(1.0f, 0.0f, 0.0f, deltaX));
			transOrigin.AddBack(Point4F::Create(0.0f, 1.0f, 0.0f, deltaY));
			transOrigin.AddBack(Point4F::Create(0.0f, 0.0f, 1.0f, deltaZ));
			transOrigin.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Build matrix: rotate on Z axis
			rotateZ.AddBack(Point4F::Create(FrogMath::Cos(ROTATION_RATE), FrogMath::Sin(-ROTATION_RATE), 0.0f, 0.0f));
			rotateZ.AddBack(Point4F::Create(FrogMath::Sin(ROTATION_RATE), FrogMath::Cos(ROTATION_RATE), 0.0f, 0.0f));
			rotateZ.AddBack(Point4F::Create(0.0f, 0.0f, 1.0f, 0.0f));
			rotateZ.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Build matrix: translate back 	
			deltaX *= -1.0f;
			deltaY *= -1.0f;
			deltaZ *= -1.0f;
			transBack.AddBack(Point4F::Create(1.0f, 0.0f, 0.0f, deltaX));
			transBack.AddBack(Point4F::Create(0.0f, 1.0f, 0.0f, deltaY));
			transBack.AddBack(Point4F::Create(0.0f, 0.0f, 1.0f, deltaZ));
			transBack.AddBack(Point4F::Create(0.0f, 0.0f, 0.0f, 1.0f));

			// Translate to origin
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(transOrigin[j]);
					}
				}
			}
			shape = M_result;

			// Rotate
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(rotateZ[j]);
					}
				}
			}
			shape = M_result;

			// Translate back
			for (int i = 0; i < shape.SizeGet(); i++)
			{
				if (shape[i] == jump)
				{
					continue;
				}
				else
				{
					for (int j = 0; j < 4; j++)
					{
						M_result[i][j] = shape[i].operator%(transBack[j]);
					}
				}
			}
			shape = M_result;

			M_result.Deinit();
			transOrigin.Deinit();
			rotateZ.Deinit();
			transBack.Deinit();
		}
	}
}
