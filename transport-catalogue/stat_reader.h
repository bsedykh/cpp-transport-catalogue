#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "transport_catalogue.h"

namespace tc {

	namespace output {

		struct Query {
			enum class Type {
				STOP,
				BUS
			};

			Type type;
			std::string name;
		};

		class Reader {
		public:
			Reader(std::istream& input);
			std::vector<Query> ReadQueries(size_t count);

		private:
			std::istream& input_;
		};

	}

	std::ostream& operator<<(std::ostream& output, const transport::StopInfo& info);
	std::ostream& operator<<(std::ostream& output, const transport::BusInfo& info);

}
