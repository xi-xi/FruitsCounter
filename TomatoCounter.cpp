#include "TomatoCounter.hpp"
#include <tuple>
#include <set>
#include "TomatoInformation.hpp"

TomatoCounter::TomatoCounter()
	:_count(0), _previous_info(){

}

void TomatoCounter::update(const std::vector<TomatoInformation>& next) {
	if (_previous_info.empty()) {
		this->_previous_info = next;
		return;
	}
	this->_previous_info = next;
}

void TomatoCounter::solveRelation(const std::vector<TomatoInformation>& next) {
	const double NEW_TOMATO_THRESH = 200;
	typedef std::tuple<double, std::size_t, std::size_t> TomatoRelation;
	std::vector<TomatoRelation> rels;
	for (std::size_t p = 0; p < this->_previous_info.size();++p) {
		for (std::size_t n = 0; n < next.size(); ++n) {
			rels.push_back(std::make_tuple(TomatoInformation::distance(this->_previous_info[p], next[n]),p,n));
		}
	}
	std::sort(rels.begin(), rels.end(),
		[](const TomatoRelation& left, const TomatoRelation& right) {
			return std::get<0>(left) > std::get<0>(right);
		}
	);
	std::set<std::size_t> used_p;
	std::set<std::size_t> used_n;
	for (const auto& r : rels) {
		if (used_p.find(std::get<1>(r)) == used_p.end()
			|| used_n.find(std::get<2>(r)) == used_n.end()) {
			continue;
		}
		else if (std::get<0>(r) > NEW_TOMATO_THRESH) {
			this->setCount(this->getCount() + 1);
		}
	}
}

std::size_t TomatoCounter::getCount() const{
	return this->_count;
}

void TomatoCounter::setCount(const std::size_t& value) {
	this->_count = value;
}