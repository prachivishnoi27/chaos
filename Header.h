#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <random>
#include <sstream>
#include <cassert>
#include <fstream>
#include "resource.h"
#include <iomanip>
#include <cmath>

//The dreaded windows include file...
#define WIN32_LEAN_AND_MEAN //Reduce compile time of windows.h
#include <Windows.h>
#undef min
#undef max

//Constants
static const int num_params = 18;
static const int iters = 800;
static const int steps_per_frame = 500;
static const double delta_per_step = 1e-5;
static const double delta_minimum = 1e-7;
static const double t_start = -3.0;
static const double t_end = 3.0;
static const bool fullscreen = false;


//Variables
static int window_w = 1600;
static int window_h = 900;
static int window_bits = 24;
static float plot_scale = 0.25f;
static float plot_x = 0.0f;
static float plot_y = 0.0f;
static std::mt19937 rand_gen;


static sf::Color GetRandColor(int i) {
	i += 1;
	int r = std::min(255, 50 + (i * 11909) % 256);
	int g = std::min(255, 50 + (i * 52973) % 256);
	int b = std::min(255, 50 + (i * 44111) % 256);
	return sf::Color(r, g, b, 16);
}

static sf::Vector2f ToScreen(double x, double y) {
	const float s = plot_scale * float(window_h / 2);
	const float nx = float(window_w) * 0.5f + (float(x) - plot_x) * s;
	const float ny = float(window_h) * 0.5f + (float(y) - plot_y) * s;
	return sf::Vector2f(nx, ny);
}

static void RandParams(double* params) {
	std::uniform_int_distribution<int> rand_int(0, 3);
	for (int i = 0; i < num_params; ++i) {
		const int r = rand_int(rand_gen);
		if (r == 0) {
			params[i] = 1.0f;
		}
		else if (r == 1) {
			params[i] = -1.0f;
		}
		else {
			params[i] = 0.0f;
		}
	}
}



static sf::RectangleShape MakeBoundsShape(const sf::Text& text) {
	sf::RectangleShape blackBox;
	const sf::FloatRect textBounds = text.getGlobalBounds();
	blackBox.setPosition(textBounds.left, textBounds.top);
	blackBox.setSize(sf::Vector2f(textBounds.width, textBounds.height));
	blackBox.setFillColor(sf::Color::Black);
	return blackBox;
}


static void ResetPlot() {
	plot_scale = 0.25f;
	plot_x = 0.0f;
	plot_y = 0.0f;
}


static void CreateRenderWindow(sf::RenderWindow& window) {
	//GL settings
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 8;
	settings.majorVersion = 3;
	settings.minorVersion = 0;

	//Create the window
	const sf::VideoMode screenSize(window_w, window_h, window_bits);
	window.create(screenSize, "The Chaos", (fullscreen ? sf::Style::Fullscreen : sf::Style::Close), settings);
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);
	window.setActive(false);
	window.requestFocus();
}

static void CenterPlot(const std::vector<sf::Vector2f>& history) {
	float min_x = FLT_MAX;
	float max_x = -FLT_MAX;
	float min_y = FLT_MAX;
	float max_y = -FLT_MAX;
	for (size_t i = 0; i < history.size(); ++i) {
		min_x = std::fmin(min_x, history[i].x);
		max_x = std::fmax(max_x, history[i].x);
		min_y = std::fmin(min_y, history[i].y);
		max_y = std::fmax(max_y, history[i].y);
	}
	max_x = std::fmin(max_x, 4.0f);
	max_y = std::fmin(max_y, 4.0f);
	min_x = std::fmax(min_x, -4.0f);
	min_y = std::fmax(min_y, -4.0f);
	plot_x = (max_x + min_x) * 0.5f;
	plot_y = (max_y + min_y) * 0.5f;
	plot_scale = 1.0f / std::max(std::max(max_x - min_x, max_y - min_y) * 0.6f, 0.1f);
}

struct Res {
	Res(int id) {
		HRSRC src = ::FindResource(NULL, MAKEINTRESOURCE(id), RT_RCDATA);
		ptr = ::LockResource(::LoadResource(NULL, src));
		size = (size_t)::SizeofResource(NULL, src);
	}
	void* ptr;
	size_t size;
};

static std::pair<double, double> CreateEquation(double x, double y,double t,double *params,float *ffi) {
	
	
	const double xx = ffi[0] * pow(x, ffi[1]);
	const double yy = ffi[2] * pow(y, ffi[3]);
	const double tt = ffi[4] * pow(t, ffi[5]);
	const double xy = ffi[6] * x * y;
	const double xt = ffi[7] * x * t;
	const double yt = ffi[8] * y * t;

	const double nx = xx * params[0] + yy * params[1] + tt * params[2] +
		xy * params[3] + xt * params[4] + yt * params[5] + x * params[6] +
		y * params[7] + t * params[8];
	const double ny = xx * params[9] + yy * params[10] + tt * params[11] +
		xy * params[12] + xt * params[13] + yt * params[14] + x * params[15] +
		y * params[16] + t * params[17];

	return std::make_pair(nx, ny);
}