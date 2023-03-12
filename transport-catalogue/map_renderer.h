#pragma once

#include <algorithm>
#include <set>
#include <vector>

#include "domain.h"
#include "svg.h"

namespace io {

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding);

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

	struct RenderSettings {
		double width = 0;
		double height = 0;
		double padding = 0.0;
		double line_width = 0.0;
		double stop_radius = 0.0;
		int bus_label_font_size = 0;
		svg::Point bus_label_offset;
		int stop_label_font_size = 0;
		svg::Point stop_label_offset;
		svg::Color underlayer_color;
		double underlayer_width = 0.0;
		std::vector<svg::Color> color_palette;
	};

	
	class MapRenderer {
        
        // ---------------------- Map objects ----------------------

        class MapObject : public svg::Drawable {
        public:
            MapObject(const SphereProjector& projector, const RenderSettings& settings);

        protected:
            const SphereProjector& projector_;
            const RenderSettings& settings_;
        };

        class BusObject : public MapObject {
        public:
            BusObject(const tc::Bus* bus, const svg::Color& color, const SphereProjector& projector, const RenderSettings& settings);

        protected:
            const tc::Bus* bus_;
            const svg::Color& color_;
        };

		class BusLine : public BusObject {
		public:
            using BusObject::BusObject;
			void Draw(svg::ObjectContainer& container) const override;		
		};

        class BusName : public BusObject {
        public:
            using BusObject::BusObject;
            void Draw(svg::ObjectContainer& container) const override;
        };

        class StopObject : public MapObject {
        public:
            StopObject(const tc::Stop* stop, const SphereProjector& projector, const RenderSettings& settings);

        protected:
            const tc::Stop* stop_;
        };

        class StopCircle : public StopObject {
        public:
            using StopObject::StopObject;
            void Draw(svg::ObjectContainer& container) const override;
        };

        class StopName : public StopObject {
        public:
            using StopObject::StopObject;
            void Draw(svg::ObjectContainer& container) const override;
        };

        // ---------------------- Misc ----------------------

        struct StopCmp {
            bool operator()(const tc::Stop* lhs, const tc::Stop* rhs) const {
                return lhs->name < rhs->name;
            }
        };

	public:
		MapRenderer(RenderSettings settings);
		svg::Document Render(std::vector<const tc::Bus*> buses) const;

	private:
		RenderSettings settings_;

        std::vector<std::pair<const tc::Bus*, const svg::Color*>> GetBusesForDrawing(std::vector<const tc::Bus*> buses) const;
        static std::set<const tc::Stop*, StopCmp> GetStopsForDrawing(const std::vector<const tc::Bus*>& buses);
        SphereProjector InitSphereProjector(const std::set<const tc::Stop*, MapRenderer::StopCmp>& stops) const;
	};

    namespace {
        const double EPSILON = 1e-6;
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }
    } // namespace

} // namespace io

// ---------------------- SphereProjector ----------------------

template <typename PointInputIt>
io::SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
    double max_width, double max_height, double padding)
    : padding_(padding) //
{
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
        return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    }
    else if (width_zoom) {
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    }
    else if (height_zoom) {
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}
