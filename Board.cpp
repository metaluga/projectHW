#include "Board.h"
#include <vector>
#include <iostream>
#include <thread>
#include <string>

using namespace sf;
using boost::asio::ip::udp;

void Board::gameCycle()
{
	
	loadSprite();
	prepareBoard();
	app.create(VideoMode(c_x, c_y), "see batle");
	sendReqest(std::string(1, command::HANDSHAKE));

	auto getData = ([&]()
	{
		getResponce();
	});


	std::thread dataRecive(getData);
	dataRecive.detach();
	

	while (app.isOpen())
	{
		Vector2i pos = Mouse::getPosition(app);
		int x = (pos.x - c_w) / c_w;
		int y = (pos.y - c_w) / c_w;

		Event e;
		while (app.pollEvent(e))
		{
			isWaite = false;
			if (e.type == Event::Closed)
			{
				app.close();
			}

			if (e.type == Event::MouseButtonPressed)
			{
				if (myMove && isGameStart && !isWaite && isOponentReady && e.key.code == Mouse::Left && x > c_boardSize)
				{
					std::cout << std::to_string(x - c_boardSize - 1) + " " + std::to_string(y) << std::endl;
					if (enemyBoard[x - c_boardSize - 1][y] == board::BOARD)
					{
						sendReqest(std::string(1, command::ATTACK) + std::to_string(x - c_boardSize - 1) + std::to_string(y));
						myMove = false;
						std::thread dataRecive(getData);
						isWaite = true;
						dataRecive.detach();
					}
				}

				if (e.key.code == Mouse::Left && !isGameStart)
				{
					
					if (y >= c_boardSize)
					{
						isGameStart = true;
						sendReqest(std::string(1, command::READY));
					}
					isShipDrag = getShip(x, y);
					if (isShipDrag != -1)
					{
						hideShip(isShipDrag, false);
					}
				}

				if (e.key.code == Mouse::Right && !isGameStart && isShipDrag != -1)
				{
					ships[isShipDrag].isHorisontal = !ships[isShipDrag].isHorisontal;
				}
			}

			if (e.type == Event::MouseButtonReleased && isShipDrag != -1)
			{
				if (e.key.code == Mouse::Left)
				{
					if (!dropShip(isShipDrag, x, y))
					{
						if (!dropShip(isShipDrag, ships[isShipDrag].x, ships[isShipDrag].y))
						{
							ships[isShipDrag].isHorisontal = !ships[isShipDrag].isHorisontal;
						}
					}
					hideShip(isShipDrag, true);
					isShipDrag = -1;
				}
			}

		}
		app.clear(Color::White);



		for (int i = 0; i < c_boardSize; ++i)
		{
			numberSprite[i].setPosition(0, i*c_w + c_w);
			app.draw(numberSprite[i]);

			numberSprite[i].setPosition((c_boardSize + 1) * c_w, i*c_w + c_w);
			app.draw(numberSprite[i]);

			letterSprite[i].setPosition(i*c_w + c_w, 0);
			app.draw(letterSprite[i]);

			letterSprite[i].setPosition(i*c_w + (c_boardSize + 2) * c_w, 0);
			app.draw(letterSprite[i]);


			for (int j = 0; j < c_boardSize; ++j)
			{
				boardSprite.setTextureRect(IntRect(mainBoard[i][j] * c_w, 0, c_w, c_w));
				boardSprite.setPosition(i*c_w + c_w, j*c_w + c_w);
				app.draw(boardSprite);

				boardSprite.setTextureRect(IntRect(enemyBoard[i][j] * c_w, 0, c_w, c_w));
				boardSprite.setPosition(i*c_w + (c_boardSize + 2) * c_w, j*c_w + c_w);
				app.draw(boardSprite);
			}

		}
		if (!isGameStart)
		{
			readySprite.setTexture(ready);
			
		}
		else if (isWin)
		{
			readySprite.setTexture(win);
		}
		else if (isLose)
		{
			readySprite.setTexture(lose);
		}
		else if (myMove && isOponentReady)
		{
			readySprite.setTexture(yourMove);
			
		}
		else
		{
			readySprite.setTexture(wait);
		}
		readySprite.setPosition(c_readyPosX, c_readyPosY);
		app.draw(readySprite);

		if (isShipDrag != -1)
		{
			moveShip(isShipDrag, pos.x, pos.y);
		}
		app.display();
	}
}

void Board::createShips()
{
	const int c_boatTypesNumber = 4;
	
	for (int i = 0; i < c_boatTypesNumber; ++i)
	{
		for (int j = i; j < c_boatTypesNumber; ++j)
		{
			Ship ship;
			ship.size = i + 1;
			ship.x = (ship.size + 1) * (j - i);
			ship.y = i * 2;
			ships.push_back(ship);
		}
	}
	
}

void Board::loadSprite()
{
	board.loadFromFile("board.jpg");
	numbers.loadFromFile("number.png");
	letter.loadFromFile("alf.jpg");
	ready.loadFromFile("ready.png");
	yourMove.loadFromFile("yourMove.png");
	wait.loadFromFile("wait.png");
	win.loadFromFile("win.png");
	lose.loadFromFile("lose.png");
	boardSprite.setTexture(board);
}

void Board::prepareBoard()
{
	for (int i = 0, k = 0; i < c_ySpriteNumber; ++i)
	{
		for (int j = 0; j < c_xSpriteNumber && k < c_boardSize; ++j)
		{
			numberSprite[k].setTexture(numbers);
			letterSprite[k].setTexture(letter);
			numberSprite[k].setTextureRect(IntRect(j*c_w, i*c_w, c_w, c_w));
			letterSprite[k].setTextureRect(IntRect(j*c_w, i*c_w, c_w, c_w));
			++k;
		}
	}

	for (int i = 0; i < c_boardSize; ++i)
	{
		for (int j = 0; j < c_boardSize; ++j)
		{
			mainBoard[i][j] = board::BOARD;
			enemyBoard[i][j] = board::BOARD;
		}
	}

	createShips();

	for (auto ship = ships.begin(); ship != ships.end(); ++ship)
	{

		for (int i = 0; i < ship->size; ++i)
		{
			if (ship->isHorisontal)
			{
				mainBoard[ship->x + i][ship->y] = board::DECK;
			}
			else
			{
				mainBoard[ship->x][ship->y + i] = board::DECK;
			}
		}

	}
}



void Board::sendReqest(std::string request)
{
	
	if (firstSetup) 
	{
		s = boost::asio::ip::udp::socket(io_context, udp::endpoint(udp::v4(), 0));
		udp::resolver resolver(io_context);
		endpoints = resolver.resolve(udp::v4(), ip, port);
		firstSetup = false;
	}
	s.send_to(boost::asio::buffer(request, request.length()), *endpoints.begin());
}

void Board::getResponce()
{
	udp::endpoint sender_endpoint;
	char reply[max_length];
	size_t reply_length = s.receive_from(
		boost::asio::buffer(reply, max_length), sender_endpoint);
	char messageType = reply[0];
	switch (messageType)
	{
	case command::START:
		if (reply[1] == '0')
		{
			std::cout << "second\n";
			myMove = false;
		}
		else
		{
			std::cout << "first\n";
			myMove = true;
		}
		if (!isOponentReady)
		{
			getResponce();
		}
		isWaite = false;
		break;
	case command::HIT:
		enemyBoard[reply[1] - '0'][reply[2] - '0'] = board::EXPLOSION;
		myMove = true;
		isWaite = false;
		break;
	case command::ATTACK:
		if (mainBoard[reply[1] - '0'][reply[2] - '0'] == board::DECK)
		{
			mainBoard[reply[1] - '0'][reply[2] - '0'] = board::EXPLOSION;
			if (isShipKiled(reply[1] - '0', reply[2] - '0'))
			{
				int getBoard = getShip(reply[1] - '0', reply[2] - '0');
				int boardX1 = ships[getBoard].x;
				int boardY1 = ships[getBoard].y;
				int boardX2 = ships[getBoard].isHorisontal ? boardX1 + ships[getBoard].size - 1 : boardX1;
				int boardY2 = ships[getBoard].isHorisontal ? boardY1 : boardY1 + ships[getBoard].size - 1;
				kiledShipProcessing(boardX1, boardY1, boardX2, boardY2, std::ref(mainBoard));
				sendReqest(std::string(1, command::KILL) +  std::to_string(boardX1) + std::to_string(boardY1) + std::to_string(boardX2) + std::to_string(boardY2));
				if (++shipIsLose == c_shipNumber)
				{
					isLose = true;
					return;
				}
			}
			else
			{
				sendReqest(std::string(1, command::HIT) + std::string(1, reply[1]) + std::string(1, reply[2]));
			}
			myMove = false;
			getResponce();
		}
		else
		{
			sendReqest(std::string(1, command::BONER) + std::string(1, reply[1]) + std::string(1, reply[2]));
			mainBoard[reply[1] - '0'][reply[2] - '0'] = board::MISS;
			myMove = true;
			isWaite = false;
		}
		break;
	case command::BONER:
		enemyBoard[reply[1] - '0'][reply[2] - '0'] = board::MISS;
		myMove = false;
		getResponce();
		break;
	case command::KILL:
		myMove = true;
		isWaite = false;
		kiledShipProcessing(reply[1] - '0', reply[2] - '0', reply[3] - '0', reply[4] - '0', std::ref(enemyBoard));
		
		if (++shipKiiled == c_shipNumber)
		{
			isWin = true;
		}
		break;
	case command::READY:
		isOponentReady = true;
		if (!myMove)
		{
			getResponce();
		}
		break;
	}
}


int Board::getShip(int x, int y)
{
	if (mainBoard[x][y] == board::DECK || mainBoard[x][y] == board::EXPLOSION)
	{
		int minX = x;
		int minY = y;
		for (int i = x; i >= 0; --i)
		{
			if (mainBoard[i][y] == board::DECK || mainBoard[i][y] == board::EXPLOSION)
			{
				minX = i;
			}
			else
			{
				break;
			}
		}

		for (int i = y; i >= 0; --i)
		{
			if (mainBoard[x][i] == board::DECK || mainBoard[x][i] == board::EXPLOSION)
			{
				minY = i;
			}
			else
			{
				break;
			}
		}

		int boardNumber = -1;
		for (int i = 0; i < ships.size(); ++i)
		{
			if (ships[i].x == minX && ships[i].y == minY)
			{
				return i;
			}
		}
		
	}
	else
	{
		return -1;
	}

	return -1;
}

void Board::moveShip(int shipNumber, int posX, int posY)
{
	
	Ship ship = ships[shipNumber];
	for (int i = 0; i < ship.size; ++i)
	{

		boardSprite.setTextureRect(IntRect(board::DECK * c_w, 0, c_w, c_w));
		if (ship.isHorisontal)
		{
			boardSprite.setPosition(posX + i * c_w - c_w / 2, posY - c_w / 2);
		}
		else
		{
			boardSprite.setPosition(posX - c_w / 2, posY + i * c_w - c_w / 2);
		}
		app.draw(boardSprite);

	}

}

void Board::hideShip(int shipNumber, bool visible)
{
	ships[shipNumber].isOnBoard = visible;
	Ship ship = ships[shipNumber];

	for (int i = 0; i < ship.size; ++i)
	{
		if (ship.isHorisontal)
		{
			mainBoard[ship.x + i][ship.y] = visible ? board::DECK : board::BOARD;
		}
		else
		{
			mainBoard[ship.x][ship.y + i] = visible ? board::DECK : board::BOARD;
		}
	}
}

bool Board::dropShip(int shipNuber, int x, int y)
{
	
	if (ships[shipNuber].isHorisontal)
	{
		if (x + ships[shipNuber].size > c_boardSize)
		{
			return false;
		}
		for (int i = x - 1; i <= x + ships[shipNuber].size; ++i)
		{
			if (y > 1 && i >= 0 && i < c_boardSize)
			{
				if (mainBoard[i][y - 1] == board::DECK)
				{
					return false;
				}

			}
			if (y < (c_boardSize - 1) && i >= 0 && i < c_boardSize)
			{
				if (mainBoard[i][y + 1] == board::DECK)
				{
					return false;
				}

			}
			if (i >= 0 && i < c_boardSize)
			{
				if (mainBoard[i][y] == board::DECK)
				{
					return false;
				}

			}
		}
	}
	else
	{
		if (y + ships[shipNuber].size > c_boardSize)
		{
			return false;
		}
		for (int i = y - 1; i <= y + ships[shipNuber].size; ++i)
		{
			if (x > 1 && i >= 0 && i < c_boardSize)
			{
				if (mainBoard[x - 1][i] == board::DECK)
				{
					return false;
				}

			}
			if (x < (c_boardSize - 1) && i >= 0 && i < c_boardSize)
			{
				if (mainBoard[x + 1][i] == board::DECK)
				{
					return false;
				}

			}
			if (i >= 0 && i < c_boardSize)
			{
				if (mainBoard[x][i] == board::DECK)
				{
					return false;
				}

			}
		}
	}

	
	ships[shipNuber].x = x;
	ships[shipNuber].y = y;

	return true;
}

bool Board::isShipKiled(int x, int y)
{

	int board = getShip(x, y);

	for (int i = 0; i < ships[board].size; ++i)
	{
		if (ships[board].isHorisontal)
		{
			if (mainBoard[ships[board].x + i][ships[board].y] == board::DECK)
			{
				return false;
			}
		}
		else
		{
			if (mainBoard[ships[board].x][ships[board].y + i] == board::DECK)
			{
				return false;
			}
		}
	}
	ships[board].isLive = false;
	return true;
}

void Board::kiledShipProcessing(int x1, int y1, int x2, int y2, std::reference_wrapper<int[c_boardSize][c_boardSize]> gameBoard)
{
	if (x1 != x2)
	{
		int startX = x1 > 0 ? x1 - 1 : 0;
		int endX = x2 < c_boardSize - 1 ? x2 + 1 : x2;
		for (int i = startX; i <= endX; ++i)
		{
			if (y1 > 0)
			{
				gameBoard[i][y1 - 1] = board::MISS;

			}
			if (y1 < c_boardSize - 1)
			{
				gameBoard[i][y1 + 1] = board::MISS;
			}
			if (x1 <= i && x2 >= i)
			{
				gameBoard[i][y1] = board::EXPLOSION;
			}
		}
		if (startX != x1)
		{
			gameBoard[startX][y1] = board::MISS;
		}
		if (endX != x2)
		{
			gameBoard[endX][y1] = board::MISS;
		}
	}
	else
	{
		int startY = y1 > 0 ? y1 - 1 : 0;
		int endY = y2 < c_boardSize - 1 ? y2 + 1 : y2;
		for (int i = startY; i <= endY; ++i)
		{
			if (x1 > 0)
			{
				gameBoard[x1 - 1][i] = board::MISS;

			}
			if (x1 < c_boardSize - 1)
			{
				gameBoard[x1 + 1][i] = board::MISS;
			}
			if (y1 <= i && y2 >= i)
			{
				gameBoard[x1][i] = board::EXPLOSION;
			}
		}
		if (startY != y1)
		{
			gameBoard[x1][startY] = board::MISS;
		}
		if (endY != y2)
		{
			gameBoard[x1][endY] = board::MISS;
		}
	}
}
