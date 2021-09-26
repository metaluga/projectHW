#pragma once
#include <SFML/Graphics.hpp>
#include <boost/asio.hpp>

struct Ship
{
	uint8_t size;
	uint8_t x, y;
	bool isHorisontal = true;
	bool isLive = true;
	bool isOnBoard = false;
};

class Board
{
public:
	Board(std::string ip, std::string port) : ip(ip), port(port), s(io_context) {};
	void gameCycle();
private:

	enum board
	{
		BOARD,
		EXPLOSION,
		MISS,
		DECK
	};
	enum command
	{
		HANDSHAKE = 'f',
		START = 's',
		HIT = 'h',
		KILL = 'k',
		BONER = 'b',
		WIN = 'w',
		LOSE = 'l',
		ATTACK = 'a',
		READY = 'r'

	};

	enum { max_length = 16 };
	enum { c_boardSize = 10 };

	sf::RenderWindow app;
	bool isGameStart = false;
	int isShipDrag = -1;
	bool myMove;

	void createShips();
	void loadSprite();
	void prepareBoard();
	void sendReqest(std::string request);
	void getResponce();
	int getShip(int x, int y);
	void moveShip(int shipNumber, int posX, int posY);
	void hideShip(int shipNumber, bool visible);
	bool dropShip(int shipNuber, int x, int y);
	bool isShipKiled(int x, int y);
	void kiledShipProcessing(int x1, int y1, int x2, int y2, std::reference_wrapper<int[c_boardSize][c_boardSize]> gameBoard);

	const int c_x = 704;
	const int c_y = 446;
	const int c_w = 32;
	

	const int c_xSpriteNumber = 4;
	const int c_ySpriteNumber = 3;
	const int c_readyPosY = 352;
	const int c_readyPosX = 0;
	const int c_shipNumber = 10;

	std::vector<Ship> ships;
	std::string ip;
	std::string port;

	boost::asio::io_context io_context;
	boost::asio::io_service io_service;
	boost::asio::ip::udp::socket s;

	bool firstSetup = true;
	bool isWaite = false;
	bool isOponentReady = false;
	bool isWin = false;
	bool isLose = false;
	int shipIsLose = 0;
	int shipKiiled = 0;

	boost::asio::ip::udp::resolver::results_type endpoints;

	int enemyBoard[c_boardSize][c_boardSize];
	int mainBoard[c_boardSize][c_boardSize];
	
	sf::Texture board, numbers, letter, ready, yourMove, wait, win, lose;
	sf::Sprite readySprite;
	sf::Sprite boardSprite;

	sf::Sprite numberSprite[c_boardSize];
	sf::Sprite letterSprite[c_boardSize];
};