#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>
#include <utility>

#include "geo.h"
#include "map_renderer.h"

using namespace std::literals;

namespace io {

	// ---------------------- SphereProjector ----------------------

	svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
		return {
			(coords.lng - min_lon_) * zoom_coeff_ + padding_,
			(max_lat_ - coords.lat) * zoom_coeff_ + padding_
		};
	}

	// ---------------------- Map objects ----------------------

	MapRenderer::MapObject::MapObject(const SphereProjector& projector, const RenderSettings& settings)
		: projector_(projector)
		, settings_(settings) {
	}

	MapRenderer::BusObject::BusObject(const tc::Bus* bus, const svg::Color& color, const SphereProjector& projector, const RenderSettings& settings)
		: MapObject(projector, settings)
		, bus_(bus)
		, color_(color) {
		assert(bus_->stops.size() > 0);
	}

	void MapRenderer::BusLine::Draw(svg::ObjectContainer& container) const {
		auto polyline = std::make_unique<svg::Polyline>();
		polyline->SetStrokeWidth(settings_.line_width);
		polyline->SetStrokeColor(color_);
		polyline->SetFillColor(svg::NoneColor);
		polyline->SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		polyline->SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		
		const auto& stops = bus_->stops;
		for (auto stop : stops) {
			polyline->AddPoint(projector_(stop->coordinates));
		}
		if (!bus_->ring) {
			auto it = stops.rbegin();
			for (++it; it != stops.rend(); ++it) {
				auto stop = *it;
				polyline->AddPoint(projector_(stop->coordinates));
			}
		}

		container.AddPtr(std::move(polyline));
	}

	void MapRenderer::BusName::Draw(svg::ObjectContainer& container) const {
		std::vector<const tc::Stop*> stops;
		stops.push_back(bus_->stops[0]);
		if (bus_->stops.front() != bus_->stops.back()) {
			stops.push_back(bus_->stops.back());
		}
		
		const auto make_text = [this](const tc::Stop* stop) {
			auto text = std::make_unique<svg::Text>();
			text->SetPosition(projector_(stop->coordinates));
			text->SetOffset(settings_.bus_label_offset);
			text->SetFontSize(settings_.bus_label_font_size);
			text->SetFontFamily("Verdana"s);
			text->SetFontWeight("bold"s);
			text->SetData(bus_->name);
			return text;
		};

		for (const tc::Stop* stop : stops) {
			auto underlayer = make_text(stop);
			underlayer->SetFillColor(settings_.underlayer_color);
			underlayer->SetStrokeColor(settings_.underlayer_color);
			underlayer->SetStrokeWidth(settings_.underlayer_width);
			underlayer->SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			underlayer->SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			container.AddPtr(std::move(underlayer));

			auto text = make_text(stop);
			text->SetFillColor(color_);
			container.AddPtr(std::move(text));
		}
	}

	MapRenderer::StopObject::StopObject(const tc::Stop* stop, const SphereProjector& projector, const RenderSettings& settings)
		: MapObject(projector, settings)
		, stop_(stop) {
	}

	void MapRenderer::StopCircle::Draw(svg::ObjectContainer& container) const {
		auto circle = std::make_unique<svg::Circle>();
		circle->SetCenter(projector_(stop_->coordinates));
		circle->SetRadius(settings_.stop_radius);
		circle->SetFillColor("white"s);
		container.AddPtr(std::move(circle));
	}

	void MapRenderer::StopName::Draw(svg::ObjectContainer& container) const {
		const auto make_text = [this]() {
			auto text = std::make_unique<svg::Text>();
			text->SetPosition(projector_(stop_->coordinates));
			text->SetOffset(settings_.stop_label_offset);
			text->SetFontSize(settings_.stop_label_font_size);
			text->SetFontFamily("Verdana"s);
			text->SetData(stop_->name);
			return text;
		};

		auto underlayer = make_text();
		underlayer->SetFillColor(settings_.underlayer_color);
		underlayer->SetStrokeColor(settings_.underlayer_color);
		underlayer->SetStrokeWidth(settings_.underlayer_width);
		underlayer->SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		underlayer->SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		container.AddPtr(std::move(underlayer));

		auto text = make_text();
		text->SetFillColor("black"s);
		container.AddPtr(std::move(text));
	}

	// ---------------------- MapRenderer ----------------------

	MapRenderer::MapRenderer(RenderSettings settings) 
		: settings_(std::move(settings)) {
	}

	svg::Document MapRenderer::Render(std::vector<const tc::Bus*> buses) const {
		svg::Document document;
		const auto buses_for_drawing = GetBusesForDrawing(buses);
		const auto stops_for_drawing = GetStopsForDrawing(buses);
		const auto projector = InitSphereProjector(stops_for_drawing);
		

		for (const auto [bus, color] : buses_for_drawing) {
			BusLine bus_line(bus, *color, projector, settings_);
			bus_line.Draw(document);
		}

		for (const auto [bus, color] : buses_for_drawing) {
			BusName bus_name(bus, *color, projector, settings_);
			bus_name.Draw(document);
		}

		for (const auto stop : stops_for_drawing) {
			StopCircle stop_circle(stop, projector, settings_);
			stop_circle.Draw(document);
		}

		for (const auto stop : stops_for_drawing) {
			StopName stop_name(stop, projector, settings_);
			stop_name.Draw(document);
		}

		return document;
	}

	std::vector<std::pair<const tc::Bus*, const svg::Color*>> MapRenderer::GetBusesForDrawing(std::vector<const tc::Bus*> buses) const {
		std::vector<std::pair<const tc::Bus*, const svg::Color*>> buses_for_drawing;

		std::sort(buses.begin(), buses.end(), [](const tc::Bus* a, const tc::Bus* b) {
			return a->name < b->name;
		});

		size_t color_index = 0;
		for (const auto bus : buses) {
			if (bus->stops.size() == 0) {
				continue;
			}

			if (color_index == settings_.color_palette.size()) {
				color_index = 0;
			}
			const svg::Color* color = &settings_.color_palette[color_index++];
			
			buses_for_drawing.emplace_back(bus, color);
		}

		return buses_for_drawing;
	}

	std::set<const tc::Stop*, MapRenderer::StopCmp> MapRenderer::GetStopsForDrawing(const std::vector<const tc::Bus*>& buses) {
		std::set<const tc::Stop*, StopCmp> stops;
		for (const auto bus : buses) {
			for (const auto stop : bus->stops) {
				stops.insert(stop);
			}
		}

		return stops;
	}

	SphereProjector MapRenderer::InitSphereProjector(const std::set<const tc::Stop*, MapRenderer::StopCmp>& stops) const {
		std::vector<geo::Coordinates> coordinates;
		coordinates.reserve(stops.size());
		for (auto stop : stops) {
			coordinates.push_back(stop->coordinates);
		}
		return SphereProjector(coordinates.begin(), coordinates.end(), settings_.width, settings_.height, settings_.padding);
	}

} // namespace io
