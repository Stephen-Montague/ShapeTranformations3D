#ifndef __MAINGAME_H__
#define __MAINGAME_H__

#include "Frog.h"
#include "MenuState.h"

namespace Webfoot {

class Sprite_X;

//==============================================================================

class MainGame : public MenuState
{
public:

   typedef MenuState Inherited;

   MainGame();
   virtual ~MainGame() {};
   
   virtual void Init();
   virtual void Deinit();

   /// Call this on every frame to update the positions.
   virtual void Update();

   /// Call this on every frame to draw the images.
   virtual void Draw();

   static MainGame instance;

protected:
   /// Returns the name of the GUI layer
   virtual const char* GUILayerNameGet();

   /// The sprite read from file [PixA.DAT]
   Sprite_X* sprite;

   /// First user input in update cycle
   char userInput;
};

MainGame* const theMainGame = &MainGame::instance;

//==============================================================================

class Sprite_X
{
public:
	Sprite_X();

	void Init();

	void Deinit();

	void Animate(char action);

	void Draw();

protected:
	// Table matrix
	Table <Point4F> M_shape;
	Table <Point4F> M_reset;
	Table <Point4F> M_draw;

	// Members to set intitial matrix
	Table <Point4F> GetShapeVertices();
	Point4F jump;
	Point4F coordinate;
};

//==============================================================================

namespace Transformation {

	void Translate(Table <Point4F> &shape, float deltaX, float deltaY, float deltaZ);

	void Scale(Table <Point4F> &shape, float scalar);

	void RotateX(Table <Point4F> &shape, float rotation);

	void RotateY(Table <Point4F> &shape, float rotation);

	void RotateZ(Table <Point4F> &shape, float rotation);
}


//==============================================================================

} //namespace Webfoot {

#endif //#ifndef __MAINGAME_H__
