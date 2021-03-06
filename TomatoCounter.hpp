#ifndef __TOMATO_COUNTER_HPP__
#define __TOMATO_COUNTER_HPP__
#include <vector>
class TomatoInformation;

class TomatoCounter
{
public:
	TomatoCounter();
	void update(const std::vector<TomatoInformation>& info);
	std::size_t getCount() const;
private:
	static double distance(const TomatoInformation& a, const TomatoInformation& b);
	std::vector<TomatoInformation> _previous_info;
	std::size_t _count;
	bool _initialized;
	void setCount(const std::size_t& value);
	void solveRelation(const std::vector<TomatoInformation>& next);
};
#endif