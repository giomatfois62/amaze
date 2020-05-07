#include "amaze.h"
#include "imgui/myimgui.h"
#include "opengl/geometry.h"

using namespace gl;
using namespace glm;

enum Directions{NORD,SUD,OVEST,EST};
enum CellStates{CURRENT=1,VISITED=2,END=3,NOTVISITED=4};

Amaze::Amaze() :
	m_grid(nullptr),
	m_wall(nullptr),
	m_gridColorBuffer(0),
	m_showMenu(true),
	centralSquare(true)
{
	setTitle("Amaze");

	ImGui::Init(sdlWindow(), context());

	m_wallShader = new Shader("./shaders/wallshader.vert",
				  "./shaders/wallshader.frag");

	m_gridShader = new Shader("./shaders/gridshader.vert",
				  "./shaders/gridshader.frag");

	Nx = Ny = 32;
	centralSquareSize = 5;

	initMaze();

	vec3 color = DEFAULT_VERTEX_COLOR;
	glClearColor(color.x, color.y, color.z, 1.0f);
}

Amaze::~Amaze()
{
	glDeleteBuffers(1, &m_gridColorBuffer);

	delete m_gridShader;
	delete m_wallShader;
	delete m_grid;
	delete m_wall;
}

void Amaze::update(float dt)
{
	updateGridColorBuffer();

	//glShadeModel(GL_FLAT);

	drawGrid();
	drawWalls();

	if (m_showMenu) {
		drawMenu();
	}
}

void Amaze::drawGrid()
{
	glUseProgram(m_gridShader->program());

	m_gridShader->setMat4("view", mat4(1.0f));
	m_gridShader->setMat4("projection", scene()->camera.projection());
	m_gridShader->setMat4("uModel", mat4(1.0f));

	glBindVertexArray(m_grid->VAO);

	glDrawElements(GL_TRIANGLES, m_grid->indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void Amaze::drawWalls()
{
	glUseProgram(m_wallShader->program());

	m_wallShader->setMat4("view", mat4(1.0f));
	m_wallShader->setMat4("projection", scene()->camera.projection());

	glBindVertexArray(m_wall->VAO);

	m_wall->updateInstancesVBO();

	glDrawElementsInstanced( GL_TRIANGLES, m_wall->indices.size(),
				 GL_UNSIGNED_INT, 0, m_wall->instances.realSize() );

	glBindVertexArray(0);
}

void Amaze::drawMenu()
{
	ImGui::RenderFrame(sdlWindow(), [&](){
		// https://github.com/ocornut/imgui/issues/1657
		ImGuiIO &io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
			ImGuiCond_Always, ImVec2(0.5f,0.5f));

		if(! ImGui::Begin("Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
			ImGui::End();
			return;
		}

		ImGui::Text("Press Arrow Keys to navigate maze.\n"
			"Press Spacebar to show/hide Options.\n"
			"Press Enter to regenerate maze."
			"\n\n"
			"RED:  Current Position\n"
			"BLUE: Maze Exit");

		ImGui::Separator();

		ImGui::InputInt("Nx", &Nx);
		ImGui::InputInt("Ny", &Ny);

		ImGui::Checkbox("Central Square", &centralSquare);
		ImGui::InputInt("Size", &centralSquareSize);

		ImGui::End();
	});
}

void Amaze::processEvent(const SDL_Event &event)
{
	ImGui::ProcessEvent(event);

	if (event.type == SDL_WINDOWEVENT) {
		if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
			updateCamera();
		}
	}

	if (event.type == SDL_MOUSEWHEEL) {
		if(event.wheel.y < 0) { // scroll up
			// TODO: zoom out

		} else if(event.wheel.y > 0) { // scroll down
			// TODO: zoom in

		}
	}

	if (event.type == SDL_KEYDOWN) {
		// move;
		m_maze(m_currentPosition.row, m_currentPosition.col) = CellStates::VISITED;

		int cellIndex = m_currentPosition.row * Ny + m_currentPosition.col;

		switch( event.key.keysym.sym ){
			case SDLK_LEFT:
				if (!m_wallMap[cellIndex][OVEST]) {
					m_currentPosition.row--;
				}
				break;
			case SDLK_RIGHT:
				if (!m_wallMap[cellIndex][EST]) {
					m_currentPosition.row++;
				}
				break;
			case SDLK_UP:
				if (!m_wallMap[cellIndex][SUD]) {
					m_currentPosition.col--;
				}
				break;
			case SDLK_DOWN:
				if (!m_wallMap[cellIndex][NORD]) {
					m_currentPosition.col++;
				}
				break;
			case SDLK_RETURN:
				initMaze();
				break;
			case SDLK_SPACE:
				m_showMenu = !m_showMenu;
				break;
			default:
				break;
		}

		m_maze(m_currentPosition.row, m_currentPosition.col) = CellStates::CURRENT;

		if (m_currentPosition == m_mazeEnd) {
			initMaze();
		}
	}
}

void Amaze::updateCamera()
{
	scene()->camera.mode = gl::Camera::ORTHO;
	scene()->camera.fitInView(xMin, xMax, yMin, yMax);
}

void Amaze::createGridColorBuffer()
{
	// recreate grid color buffer
	if (m_gridColorBuffer)
		glDeleteBuffers(1, &m_gridColorBuffer);

	glGenBuffers(1, &m_gridColorBuffer);
	glBindVertexArray(m_grid->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_gridColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_maze.rows() * m_maze.cols() * sizeof(float),
		     m_maze.data(), GL_STREAM_DRAW);

	glEnableVertexAttribArray(9);
	glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, sizeof(float),
			      (void*)0);

	glBindVertexArray(0);
}

void Amaze::updateGridColorBuffer()
{
	// update maze VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_gridColorBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * m_maze.rows() * m_maze.cols(),
			m_maze.data());
}

void Amaze::initMaze()
{
	// create matrices
	Matrix2F maze(Nx, Ny);
	Matrix2F directions(Nx,Ny);

	float aspect = Nx / (float)Ny;

	xMin = -aspect / 2;
	xMax = aspect / 2;
	yMin = -.5;
	yMax = .5;

	maze.fill(0);
	directions.fill(0);

	m_maze = Matrix2F(Nx+1, Ny+1);
	m_maze.fill(CellStates::NOTVISITED);

	// create grid mesh
	if (m_grid)
		delete m_grid;

	m_grid = grid(xMin, xMax, yMin, yMax, Nx+1, Ny+1);

	// create wall mesh
	if (m_wall)
		delete m_wall;

	m_wall = cube();

	// add wall instance to each cell
	float dx = (xMax -xMin) / (Nx);
	float dy = (yMax -yMin) / (Ny);
	float thicness = 0.1;

	m_wallMap.clear();
	m_wallMap = std::vector<std::array<bool,4>>(Nx*Ny, {true,true,true,true});

	for (size_t i = 0; i < maze.rows(); ++i) {
		for (size_t j = 0; j < maze.cols(); ++j) {
			float x = xMin + i *dx;
			float y = yMin + j *dy;

			// NORD
			{
				mat4 inst(1.0f);
				inst = glm::translate(inst, vec3(x + dx/2,y + dy,0));
				inst = glm::scale(inst, vec3( dx + dx*thicness, dy * thicness, min(dx,dy)));

				m_wall->instances.insert(inst);
			}

			// SUD
			{
				mat4 inst(1.0f);
				inst = glm::translate(inst, vec3(x + dx/2,y,0));
				inst = glm::scale(inst, vec3( dx + dx*thicness, dy * thicness, min(dx,dy)));

				m_wall->instances.insert(inst);

			}
			// OVEST
			{
				mat4 inst(1.0f);
				inst = glm::translate(inst, vec3(x,y+dy/2,0));
				inst = glm::scale(inst, vec3(  dx*thicness, dy + dy*thicness, min(dx,dy)));

				m_wall->instances.insert(inst);
			}


			// EST
			{
				mat4 inst(1.0f);
				inst = glm::translate(inst, vec3(x + dx,y+dy/2,0));
				inst = glm::scale(inst, vec3( dx*thicness, dy + dy*thicness, min(dx,dy)));

				m_wall->instances.insert(inst);
			}
		}
	}

	auto isInMaze = [&](int i, int j) {
		return maze(i,j) > 0;
	};

	auto removeWallInstance = [&](int i, int j, int dir) {
		int index = i * maze.cols() * 4 + j * 4 + dir;

		m_wall->instances[index] = mat4(0.0f);

		m_wallMap[i * maze.cols() + j][dir] = false;
	};

	auto moveCell = [](int &i, int &j, int dir) {
		switch(dir) {
			case EST:
				i = i+1; // EST
				break;
			case NORD:
				j = j+1; // NORD
				break;
			case OVEST:
				i = i-1; // OVEST
				break;
			case SUD:
				j = j-1; // SUD
				break;
		}
	};

	auto randomWalk = [&](int &i, int &j) {
		std::vector<int> dirs;

		if (i > 0) dirs.push_back(OVEST);
		if (j > 0) dirs.push_back(SUD);
		if (i < (int)maze.rows() - 1) dirs.push_back(EST);
		if (j < (int)maze.cols() - 1) dirs.push_back(NORD);

		size_t p = rand() % dirs.size();

		directions(i,j) = dirs[p];

		moveCell(i, j, dirs[p]);
	};

	auto getNextCell = [&](int &ii, int &jj) {
		for (size_t i = 0; i < maze.rows(); ++i) {
			for (size_t j = 0; j < maze.cols(); ++j) {
				if (!isInMaze(i,j)) {
					ii = i;
					jj = j;

					goto cellfound;
				}
			}
		}

		return false;

		cellfound:
			return true;
	};

	// select start/end points and remove their walls
	auto selectRandomEnd = [&]() {
		int rnd = rand()%maze.cols();

		return Cell(maze.rows()-1, rnd);
	};

	auto selectRandomStart = [&] {
		int rnd = rand()%maze.cols();

		return Cell(0, rnd);
	};

	m_mazeEnd = selectRandomEnd();
	removeWallInstance(m_mazeEnd.row, m_mazeEnd.col,EST);

	m_currentPosition = selectRandomStart();
	removeWallInstance(m_currentPosition.row, m_currentPosition.col, OVEST);

	// generate maze
	int i = m_currentPosition.row;
	int j = m_currentPosition.col;

	if (!centralSquare || centralSquareSize <= 0) {
		// insert first cell in maze
		maze(m_mazeEnd.row, m_mazeEnd.col) = 1;
	} else {
		// insert central square
		int startRow = Nx/2.0f - centralSquareSize/2.0f;
		int startCol = Ny/2.0f - centralSquareSize/2.0f;

		for (int row = startRow; row < startRow + centralSquareSize; ++row ) {
			for (int col = startCol; col < startCol + centralSquareSize; ++col) {
				maze(row, col) = 1;
			}
		}

		// remove walls
		for (int row = startRow; row < startRow + centralSquareSize; ++row ) {
			for (int col = startCol; col < startCol + centralSquareSize; ++col) {
				if (maze(row+1, col))
					removeWallInstance(row,col,EST);
				if (maze(row, col+1))
					removeWallInstance(row,col,NORD);
				if (maze(row-1, col))
					removeWallInstance(row,col,OVEST);
				if (maze(row, col-1))
					removeWallInstance(row,col,SUD);
			}
		}
	}

	do {
		int i0 = i;
		int j0 = j;

		while(!isInMaze(i,j)) {
			randomWalk(i,j);
		}

		int backDir;

		while(!isInMaze(i0, j0)) {
			// remove walls
			int dir = directions(i0, j0);
			removeWallInstance(i0,j0,dir);

			maze(i0,j0) = 1;

			moveCell(i0, j0, directions(i0, j0));

			switch(dir) {
				case NORD:
					backDir = SUD;
					break;
				case SUD:
					backDir = NORD;
					break;
				case EST:
					backDir = OVEST;
					break;
				case OVEST:
					backDir = EST;
					break;
			}

			removeWallInstance(i0,j0,backDir);
		}

	} while(getNextCell(i, j));

	m_maze(m_currentPosition.row, m_currentPosition.col) = CellStates::CURRENT;
	m_maze(m_mazeEnd.row, m_mazeEnd.col) = CellStates::END;

	createGridColorBuffer();
	updateGridColorBuffer();

	updateCamera();
}
