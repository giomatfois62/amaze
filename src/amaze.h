#ifndef AMAZE_H
#define AMAZE_H

#include <array>
#include "opengl/opengl.h"
#include "matrix.h"

typedef Matrix<float, 2> Matrix2F;

struct Cell {
	int row;
	int col;

	Cell() {}
	Cell(int row, int col) : row(row), col(col) {}

	friend bool operator==(const Cell &p1, const Cell &p2)
	{
		return p1.row == p2.row && p1.col == p2.col;
	}
};

class Amaze : public gl::OpenGLWindow {
	public:
		Amaze();
		~Amaze();

	protected:
		void update(float dt);
		void processEvent(const SDL_Event &event);

		void drawGrid();
		void drawWalls();
		void drawMenu();

		void initMaze();
		void updateCamera();

		void createGridColorBuffer();
		void updateGridColorBuffer();

		gl::Mesh *m_grid;
		gl::Mesh *m_wall;
		gl::Shader *m_wallShader;
		gl::Shader *m_gridShader;
		GLuint m_gridColorBuffer;

		float xMin, xMax, yMin, yMax;
		int Nx,Ny;
		bool m_showMenu;
		bool centralSquare;
		int centralSquareSize;

		Matrix2F m_maze;
		std::vector<std::array<bool, 4>> m_wallMap;

		Cell m_currentPosition;
		Cell m_mazeEnd;
};

#endif
