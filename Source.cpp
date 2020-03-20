#include "Header.h"

int main(int argc, char* argv[]) {

	//Set random seed
	rand_gen.seed((unsigned int)time(0));

	
	//Create the window
	const sf::VideoMode screenSize = sf::VideoMode::getDesktopMode();
	window_bits = screenSize.bitsPerPixel;
	if (fullscreen) {
		window_w = screenSize.width;
		window_h = screenSize.height;
	}
	sf::RenderWindow window;
	CreateRenderWindow(window);

	//Simulation variables
	double t = t_start;
	std::vector<sf::Vector2f> history(iters);
	double rolling_delta = delta_per_step;
	double params[num_params];
	double speed_mult = 1.0;
	bool paused = false;
	int trail_type = 0;
	int dot_type = 0;
	bool shuffle_equ = true;
	bool iteration_limit = false;

	//Setup the vertex array
	std::vector<sf::Vertex> vertex_array(iters * steps_per_frame);
	for (size_t i = 0; i < vertex_array.size(); ++i) {
		vertex_array[i].color = GetRandColor(i % iters);
	}
 
	//Initialize random parameters
	ResetPlot();
	RandParams(params);

	float ffi[9];

	std::cout << "Enter the coefficient and exponential of x, y and t in pairs and only coefficient of rest:" << std::endl;
	std::cout << std::setw(3) << "x" << std::setw(3) << "y" << std::setw(3) << "t"
		<< std::setw(3) << "xy" << std::setw(3) << "xt" << std::setw(3) << "yt"
		<< std::endl;
	std::cin >> ffi[0] >> ffi[1] >> ffi[2] >> ffi[3] >> ffi[4] >> ffi[5] >> ffi[6] >> ffi[7] >> ffi[8];


	//Main Loop
	while (true) {
		while (window.isOpen()) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					window.close();
					break;
				}
				else if (event.type == sf::Event::KeyPressed) {
					const sf::Keyboard::Key keycode = event.key.code;
					if (keycode == sf::Keyboard::Escape) {
						window.close();
						break;
					}
					else if (keycode == sf::Keyboard::A) {
						shuffle_equ = true;
					}
					else if (keycode == sf::Keyboard::C) {
						CenterPlot(history);
					}
					else if (keycode == sf::Keyboard::D) {
						dot_type = (dot_type + 1) % 3;
					}
					else if (keycode == sf::Keyboard::I) {
						iteration_limit = !iteration_limit;
					}
					else if (keycode == sf::Keyboard::N) {
						ResetPlot();
						RandParams(params);
						
					}
					else if (keycode == sf::Keyboard::P) {
						paused = !paused;
					}
					else if (keycode == sf::Keyboard::R) {
						shuffle_equ = false;
					}
					else if (keycode == sf::Keyboard::T) {
						trail_type = (trail_type + 1) % 4;
					}
				}
			}

			//Change simulation speed if using shift modifiers
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
				speed_mult = 0.1;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
				speed_mult = 10.0;
			}
			else {
				speed_mult = 1.0;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
				speed_mult = -speed_mult;
			}

			//Skip all drawing if paused
			if (paused) {
				window.display();
				continue;
			}

			//Automatic restart
			if (t > t_end) {
				if (shuffle_equ) {
					ResetPlot();
					RandParams(params);
				}
			}

			sf::BlendMode fade(sf::BlendMode::One, sf::BlendMode::One, sf::BlendMode::ReverseSubtract);
			sf::RenderStates renderBlur(fade);

			sf::RectangleShape fullscreen_rect;
			fullscreen_rect.setPosition(0.0f, 0.0f);
			fullscreen_rect.setSize(sf::Vector2f(window_w, window_h));

			static const sf::Uint8 fade_speeds[] = { 10,2,0,255 };
			const sf::Uint8 fade_speed = fade_speeds[trail_type];
			if (fade_speed >= 1) {
				fullscreen_rect.setFillColor(sf::Color(fade_speed, fade_speed, fade_speed, 0));
				window.draw(fullscreen_rect, renderBlur);
			}

			//Smooth out the stepping speed.
			const int steps = steps_per_frame;
			const double delta = delta_per_step * speed_mult;
			rolling_delta = rolling_delta * 0.99 + delta * 0.01;

			//Apply chaos
			for (int step = 0; step < steps; ++step) {
				bool isOffScreen = true;
				double x = t;
				double y = t;

				
				for (int iter = 0; iter < iters; ++iter) {

					std::pair<double, double> toPoint = CreateEquation(x, y, t, params,ffi);
					x = toPoint.first;
					y = toPoint.second;

					sf::Vector2f screenPt = ToScreen(x, y);
					if (iteration_limit && iter < 100) {
						screenPt.x = FLT_MAX;
						screenPt.y = FLT_MAX;
					}
					vertex_array[step * iters + iter].position = screenPt;

					//Check if dynamic delta should be adjusted
					if (screenPt.x > 0.0f && screenPt.y > 0.0f && screenPt.x < window_w && screenPt.y < window_h) {
						const float dx = history[iter].x - float(x);
						const float dy = history[iter].y - float(y);
						const double dist = double(500.0f * std::sqrt(dx * dx + dy * dy));
						rolling_delta = std::min(rolling_delta, std::max(delta / (dist + 1e-5), delta_minimum * speed_mult));
						isOffScreen = false;
					}
					history[iter].x = float(x);
					history[iter].y = float(y);
				}

				//Update the t variable
				if (isOffScreen) {
					t += 0.01;
				}
				else {
					t += rolling_delta;
				}
			}

			//Draw new points
			static const float dot_sizes[] = { 1.0f, 3.0f, 10.0f };
			glEnable(GL_POINT_SMOOTH);
			glPointSize(dot_sizes[dot_type]);
			window.draw(vertex_array.data(), vertex_array.size(), sf::PrimitiveType::Points);

			//Flip the screen buffer
			window.display();
		}
	}
	return 0;
}


