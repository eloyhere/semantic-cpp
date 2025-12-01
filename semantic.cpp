#include "semantic.h"
namespace semantic
{
template <typename D>
D randomly(const D &start, const D &end)
{
	static std::random_device rd;
	static std::mt19937_64 engine(rd());

	if constexpr (std::is_integral_v<D> && !std::is_same_v<D, bool>)
	{
		std::uniform_int_distribution<D> dist(start, end);
		return dist(engine);
	}
	else if constexpr (std::is_floating_point_v<D>)
	{
		std::uniform_real_distribution<D> dist(start, end);
		return dist(engine);
	}
	else if constexpr (std::is_same_v<D, bool>)
	{
		std::bernoulli_distribution dist(0.5);
		return dist(engine);
	}
	else
	{
		static_assert(std::is_arithmetic_v<D>, "D must be an arithmetic type");
		return D{};
	}
}

//Statistics implementaion starts here.
template <typename E, typename D>
Module Statistics<E, D>::count() const
{
	return size;
}

template <typename E, typename D>
D Statistics<E, D>::maximum() const
{
	if (isEmpty())
		return D{};
	auto it = cache.find("maximum");
	if (it != cache.end())
		return it->second;

	D maxVal = elements[0];
	for (Module i = 1; i < size; ++i)
	{
		if (elements[i] > maxVal)
		{
			maxVal = elements[i];
		}
	}
	cache["maximum"] = maxVal;
	return maxVal;
}

template <typename E, typename D>
D Statistics<E, D>::minimum() const
{
	if (isEmpty())
		return D{};
	auto it = cache.find("minimum");
	if (it != cache.end())
		return it->second;

	D minVal = elements[0];
	for (Module i = 1; i < size; ++i)
	{
		if (elements[i] < minVal)
		{
			minVal = elements[i];
		}
	}
	cache["minimum"] = minVal;
	return minVal;
}

template <typename E, typename D>
D Statistics<E, D>::range() const
{
	if (isEmpty())
		return D{};
	auto it = cache.find("range");
	if (it != cache.end())
		return it->second;

	D rangeVal = maximum() - minimum();
	cache["range"] = rangeVal;
	return rangeVal;
}

template <typename E, typename D>
D Statistics<E, D>::variance() const
{
	if (isEmpty() || size == 1)
		return D{};
	auto it = cache.find("variance");
	if (it != cache.end())
		return it->second;

	D meanVal = mean();
	D varianceVal = D{};
	for (Module i = 0; i < size; ++i)
	{
		D diff = elements[i] - meanVal;
		varianceVal += diff * diff;
	}
	varianceVal /= static_cast<D>(size - 1);
	cache["variance"] = varianceVal;
	return varianceVal;
}

template <typename E, typename D>
D Statistics<E, D>::standardDeviation() const
{
	if (isEmpty())
		return D{};
	auto it = cache.find("standardDeviation");
	if (it != cache.end())
		return it->second;

	D stdDev = std::sqrt(variance());
	cache["standardDeviation"] = stdDev;
	return stdDev;
}

template <typename E, typename D>
D Statistics<E, D>::mean() const
{
	if (isEmpty())
		return D{};
	auto it = cache.find("mean");
	if (it != cache.end())
		return it->second;

	D meanVal = sum() / static_cast<D>(size);
	cache["mean"] = meanVal;
	return meanVal;
}

template <typename E, typename D>
D Statistics<E, D>::median() const
{
	if (isEmpty())
		return D{};
	auto it = cache.find("median");
	if (it != cache.end())
		return it->second;

	std::vector<D> sorted(elements, elements + size);
	std::sort(sorted.begin(), sorted.end());
	D medianVal;
	if (size % 2 == 0)
	{
		medianVal = (sorted[size / 2 - 1] + sorted[size / 2]) / 2;
	}
	else
	{
		medianVal = sorted[size / 2];
	}
	cache["median"] = medianVal;
	return medianVal;
}

template <typename E, typename D>
D Statistics<E, D>::mode() const
{
	if (isEmpty())
		return D{};
	auto it = cache.find("mode");
	if (it != cache.end())
		return it->second;

	auto freq = frequency();
	D modeVal = elements[0];
	Module maxCount = 0;
	for (const auto &pair : freq)
	{
		if (pair.second > maxCount)
		{
			maxCount = pair.second;
			modeVal = pair.first;
		}
	}
	cache["mode"] = modeVal;
	return modeVal;
}

template <typename E, typename D>
std::map<D, Module> Statistics<E, D>::frequency() const
{
	if (!frequencyCache.empty())
		return frequencyCache;

	for (Module i = 0; i < size; ++i)
	{
		frequencyCache[static_cast<D>(elements[i])]++;
	}
	return frequencyCache;
}

template <typename E, typename D>
D Statistics<E, D>::sum() const
{
	if (isEmpty())
		return D{};
	auto it = cache.find("sum");
	if (it != cache.end())
		return it->second;

	D sumVal = std::accumulate(elements, elements + size, D{});
	cache["sum"] = sumVal;
	return sumVal;
}

template <typename E, typename D>
std::vector<D> Statistics<E, D>::quartiles() const
{
	if (isEmpty())
		return {};
	auto it = cache.find("quartiles");
	if (it != cache.end())
		return it->second;

	std::vector<D> sorted(elements, elements + size);
	std::sort(sorted.begin(), sorted.end());

	std::vector<D> quartiles(3);
	Module n = size;

	quartiles[0] = sorted[n / 4];
	quartiles[1] = sorted[n / 2];
	quartiles[2] = sorted[3 * n / 4];

	cache["quartiles"] = quartiles;
	return quartiles;
}

template <typename E, typename D>
D Statistics<E, D>::interquartileRange() const
{
	if (isEmpty())
		return D{};
	auto it = cache.find("interquartileRange");
	if (it != cache.end())
		return it->second;

	auto q = quartiles();
	D iqr = q[2] - q[0];
	cache["interquartileRange"] = iqr;
	return iqr;
}

template <typename E, typename D>
D Statistics<E, D>::skewness() const
{
	if (isEmpty() || size < 3)
		return D{};
	auto it = cache.find("skewness");
	if (it != cache.end())
		return it->second;

	D meanVal = mean();
	D stdDev = standardDeviation();
	D skew = D{};
	for (Module i = 0; i < size; ++i)
	{
		D diff = (elements[i] - meanVal) / stdDev;
		skew += diff * diff * diff;
	}
	skew /= static_cast<D>(size);
	cache["skewness"] = skew;
	return skew;
}

template <typename E, typename D>
D Statistics<E, D>::kurtosis() const
{
	if (isEmpty() || size < 4)
		return D{};
	auto it = cache.find("kurtosis");
	if (it != cache.end())
		return it->second;

	D meanVal = mean();
	D stdDev = standardDeviation();
	D kurt = D{};
	for (Module i = 0; i < size; ++i)
	{
		D diff = (elements[i] - meanVal) / stdDev;
		kurt += diff * diff * diff * diff;
	}
	kurt = kurt / static_cast<D>(size) - 3;
	cache["kurtosis"] = kurt;
	return kurt;
}

template <typename E, typename D>
bool Statistics<E, D>::isEmpty() const
{
	return size == 0;
}

template <typename E, typename D>
void Statistics<E, D>::clear()
{
	cache.clear();
	frequencyCache.clear();
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(const std::list<E> &l)
{
	clear();
	std::vector<E> temp(l.begin(), l.end());
	const_cast<E *&>(elements) = new E[temp.size()];
	std::copy(temp.begin(), temp.end(), const_cast<E *>(elements));
	const_cast<Module &>(size) = temp.size();
	return *this;
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(const std::vector<E> &v)
{
	clear();
	const_cast<E *&>(elements) = const_cast<E *>(v.data());
	const_cast<Module &>(size) = v.size();
	return *this;
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(std::initializer_list<E> l)
{
	clear();
	const_cast<E *&>(elements) = const_cast<E *>(l.begin());
	const_cast<Module &>(size) = l.size();
	return *this;
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(const Statistics &other)
{
	if (this != &other)
	{
		clear();
		const_cast<E *&>(elements) = other.elements;
		const_cast<Module &>(size) = other.size;
		cache = other.cache;
		frequencyCache = other.frequencyCache;
	}
	return *this;
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(Statistics &&other) noexcept
{
	if (this != &other)
	{
		clear();
		const_cast<E *&>(elements) = other.elements;
		const_cast<Module &>(size) = other.size;
		cache = std::move(other.cache);
		frequencyCache = std::move(other.frequencyCache);
		const_cast<E *&>(other.elements) = nullptr;
		const_cast<Module &>(other.size) = 0;
	}
	return *this;
}
//Statistics implementaion ends here.

//Semantic implementation starts here.

template <typename E>
Semantic<E> &Semantic<E>::operator=(const Semantic<E> &other)
{
	if (this != &other)
	{
		generator = other.generator;
		concurrent = other.concurrent;
	}
	return *this;
}

template <typename E>
Semantic<E> &Semantic<E>::operator=(Semantic<E> &&other) noexcept
{
	if (this != &other)
	{
		generator = std::move(other.generator);
		concurrent = other.concurrent;
	}
	return *this;
}

template <typename E>
bool Semantic<E>::allMatch(const Predicate<E> &predicate) const
{
	bool mismatch = true;
	(*generator)([&mismatch, &predicate](const E &element) -> void {
            if (!predicate(element)) {
                mismatch = true;
            } }, [&mismatch](const E &element) -> bool { return mismatch; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	return mismatch;
}

template <typename E>
bool Semantic<E>::anyMatch(const Predicate<E> &predicate) const
{
	bool anyMatch = false;
	(*generator)([&anyMatch, &predicate](const E &element) -> void {
            if (predicate(element)) {
                anyMatch = true;
            } }, [&anyMatch](const E &element) -> bool { return anyMatch; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	return anyMatch;
}

template <typename E>
template <typename A, typename R>
R Semantic<E>::collect(const Supplier<A> &supplier, const BiConsumer<A, E> &accumulator, const BiFunction<A, A, A> &combiner, const Function<A, R> &finisher) const
{
	A container = supplier();
	(*generator)([&container, &accumulator](const E &element) -> void { accumulator(container, element); }, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	return finisher(container);
}

template <typename E>
template <typename A, typename R>
R Semantic<E>::collect(const Collector<E, A, R> &collector) const
{
	return collect<A, R>(collector.supplier, collector.accumulator, collector.combiner, collector.finisher);
}

template <typename E>
Semantic<E> Semantic<E>::concat(const Semantic<E> &other) const
{
	return Semantic<E>(std::make_shared<Generator<E>>([this, other](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		bool firstCompleted = false;
		(*generator)(
			accept, [&](const E &element) -> bool {
				if (interrupt(element))
				{
					firstCompleted = true;
					return false;
				}
				return true;
			},
			redirect);

		if (!firstCompleted)
		{
			(*other.generator)(accept, interrupt, redirect);
		}
	}));
}

template <typename E>
void Semantic<E>::cout() const
{
	bool first = true;
	std::cout << '[';
	(*generator)([&first](const E &element) -> void { 
	    if(first){
	    	first = false;
	    }else{
	    	std::cout << ',';
	    }
	    std::cout << element; }, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	std::cout << ']';
}

template <typename E>
void Semantic<E>::cout(const BiFunction<E, std::ostream &, std::ostream &> &formatter) const
{
	(*generator)([&formatter](const E &element) -> void { formatter(element, std::cout); }, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	std::cout << std::endl;
}

template <typename E>
void Semantic<E>::cout(const std::ostream &stream) const
{
	std::ostream &output = const_cast<std::ostream &>(stream);
	(*generator)([&output](const E &element) -> void { output << element << " "; }, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	output << std::endl;
}

template <typename E>
void Semantic<E>::cout(const std::ostream &stream, const BiFunction<E, std::ostream &, std::ostream &> &formatter) const
{
	std::ostream &output = const_cast<std::ostream &>(stream);
	(*generator)([&output, &formatter](const E &element) -> void { formatter(element, output); }, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	output << std::endl;
}

template <typename E>
Module Semantic<E>::count() const
{
	Module count = 0;
	(*generator)([&count](const E &element) -> void { count++; }, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	return count;
}

template <typename E>
Semantic<E> Semantic<E>::distinct() const
{
	return Semantic<E>(std::make_shared<Generator<E>>([this](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) -> void {
		std::unordered_set<E> seen;
		(*generator)([&](const E &element) -> void {
			if (seen.find(element) == seen.end())
			{
				seen.insert(element);
				if (accept)
					accept(element);
			}
		},
					 interrupt, redirect);
	}));
}

template <typename E>
Semantic<E> Semantic<E>::distinct(const BiPredicate<E, E> &comparator) const
{
	return Semantic<E>(std::make_shared<Generator<E>>([this, comparator](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) -> void {
		std::vector<E> seen;
		(*generator)([&](const E &element) -> void {
			bool isDuplicate = false;
			for (const auto &existing : seen)
			{
				if (comparator(element, existing))
				{
					isDuplicate = true;
					break;
				}
			}
			if (!isDuplicate)
			{
				seen.push_back(element);
				if (accept)
					accept(element);
			}
		},
					 interrupt, redirect);
	}));
}

template <typename E>
Semantic<E> Semantic<E>::dropWhile(const Predicate<E> &predicate) const
{
	bool dropping = true;
	return Semantic<E>(std::make_shared<Generator<E>>([this, predicate, dropping](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) mutable {
		(*generator)([&](const E &element) -> void {
			if (dropping)
			{
				if (!predicate(element))
				{
					dropping = false;
					if (accept)
						accept(element);
				}
			}
			else
			{
				if (accept)
					accept(element);
			}
		},
					 interrupt, redirect);
	}));
}

template <typename E>
void Semantic<E>::forEach(const Consumer<E> &consumer) const
{
	(*generator)(
		consumer, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
}

template <typename E>
Semantic<E> Semantic<E>::filter(const Predicate<E> &predicate) const
{
	return Semantic<E>(std::make_shared<Generator<E>>([this, predicate](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		(*generator)([&](const E &element) -> void {
                if (predicate(element) && accept) {
                    accept(element);
                } }, interrupt, redirect);
	}));
}

template <typename E>
std::optional<E> Semantic<E>::findFirst() const
{
	std::optional<E> result;
	(*generator)([&result](const E &element) -> void {
            if (!result) {
                result = element;
            } }, [&result](const E &element) -> bool { return result; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	return result;
}

template <typename E>
std::optional<E> Semantic<E>::findAny() const
{
	return findFirst();
}

template <typename E>
Semantic<E> Semantic<E>::flat(const Function<E, Semantic<E>> &mapper) const
{
	return Semantic<E>(std::make_shared<Generator<E>>([this, mapper](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		(*generator)([&](const E &element) -> void {
			auto nestedStream = mapper(element);
			(*nestedStream.generator)(accept, interrupt, redirect);
		},
					 interrupt, redirect);
	}));
}

template <typename E>
template <typename R>
Semantic<R> Semantic<E>::flatMap(const Function<E, Semantic<R>> &mapper) const
{
	return Semantic<R>(std::make_shared<Generator<R>>([this, mapper](const Consumer<R> &accept, const Predicate<R> &interrupt, const BiFunction<R, Timestamp, Timestamp> &redirect) {
		(*generator)([&](const E &element) -> void {
                auto nestedStream = mapper(element);
                (*nestedStream.generator)(accept, interrupt, redirect); }, [](const E &element) -> bool { return false; }, [](const E &element, Timestamp ts) -> Timestamp { return ts; });
	}));
}

template <typename E>
bool Semantic<E>::noneMatch(const Predicate<E> &predicate) const
{
	return !anyMatch(predicate);
}

template <typename E>
template <typename K>
std::map<K, std::vector<E>> Semantic<E>::group(const Function<E, K> &classifier) const
{
	std::map<K, std::vector<E>> groups;
	(*generator)([&groups, &classifier](const E &element) -> void {
            K key = classifier(element);
            groups[key].push_back(element); }, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	return groups;
}

template <typename E>
template <typename K, typename V>
std::map<K, std::vector<V>> Semantic<E>::groupBy(const Function<E, K> &keyExtractor, const Function<E, V> &valueExtractor) const
{
	std::map<K, std::vector<V>> groups;
	(*generator)([&groups, &keyExtractor, &valueExtractor](const E &element) -> void {
            K key = keyExtractor(element);
            V value = valueExtractor(element);
            groups[key].push_back(value); }, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	return groups;
}

template <typename E>
Semantic<E> Semantic<E>::limit(const Module &n) const
{
	Module count = 0;
	return Semantic<E>(std::make_shared<Generator<E>>([this, n, count](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) mutable {
		(*generator)([&](const E &element) -> void { accept(element); }, [&](const E &element) -> bool { return count >= n || interrupt(element); }, redirect);
	}));
}

template <typename E>
template <typename R>
Semantic<R> Semantic<E>::map(const Function<E, R> &mapper) const
{
	return Semantic<R>(std::make_shared<Generator<R>>([this, mapper](const Consumer<R> &accept, const Predicate<R> &interrupt, const BiFunction<R, Timestamp, Timestamp> &redirect) -> void {
		(*generator)([&](const E &element) -> void {
                R result = mapper(element);
                if (accept) accept(result); }, [&](const E &element) -> bool { return interrupt(mapper(element)); }, [&](const E &element, Timestamp ts) -> Timestamp {
                        R result = mapper(element);
                        return redirect ? redirect(result, ts) : ts; });
	}));
}

template <typename E>
Semantic<E> Semantic<E>::parallel() const
{
	auto newSemantic = *this;
	newSemantic.parallelable = true;
	return newSemantic;
}

template <typename E>
Semantic<E> Semantic<E>::parallel(const Module &threadCount) const
{
	auto newSemantic = *this;
	newSemantic.parallelable = true;
	return newSemantic;
}

template <typename E>
std::vector<std::vector<E>> Semantic<E>::partition(const Module &count) const
{
	std::vector<std::vector<E>> partitions;
	std::vector<E> currentPartition;
	Module currentCount = 0;

	(*generator)([&](const E &element) -> void {
            currentPartition.push_back(element);
            currentCount++;
            if (currentCount >= count) {
                partitions.push_back(currentPartition);
                currentPartition.clear();
                currentCount = 0;
            } }, [](const E &element) -> bool { return true; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });

	if (!currentPartition.empty())
	{
		partitions.push_back(currentPartition);
	}
	return partitions;
}

template <typename E>
std::vector<std::vector<E>> Semantic<E>::partitionBy(const Function<E, Module> &classifier) const
{
	std::vector<std::vector<E>> partitions;
	std::map<Module, std::vector<E>> partitionMap;

	(*generator)([&](const E &element) -> void {
            Module partitionKey = classifier(element);
            partitionMap[partitionKey].push_back(element); }, [](const E &element) -> bool { return true; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });

	for (auto &pair : partitionMap)
	{
		partitions.push_back(std::move(pair.second));
	}
	return partitions;
}

template <typename E>
Semantic<E> Semantic<E>::peek(const Consumer<E> &consumer) const
{
	return Semantic(std::make_shared<Generator<E>>([this, consumer](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		(*generator)([&](const E &element) -> void {
			consumer(element);
			accept(element);
		},
					 interrupt, redirect);
	}));
}

template <typename E>
Semantic<E> Semantic<E>::redirect(const BiFunction<E, Timestamp, Timestamp> &redirector) const
{
	return Semantic<E>(std::make_shared<Generator<E>>([this, redirector](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		(*generator)(accept, interrupt, [&redirect, &redirector](const E &element, const Timestamp &index) -> Timestamp {
			return redirector(element, redirect(element, index));
		});
	}));
}

template <typename E>
std::optional<E> Semantic<E>::reduce(const BiFunction<E, E, E> &accumulator) const
{
	std::optional<E> result;
	(*generator)([&result, &accumulator](const E &element) -> void {
            if (!result) {
                result = element;
            }
            else {
                result = accumulator(*result, element);
            } }, [](const E &element) -> bool { return true; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	return result;
}

template <typename E>
E Semantic<E>::reduce(const E &identity, const BiFunction<E, E, E> &accumulator) const
{
	E result = identity;
	(*generator)([&result, &accumulator](const E &element) -> void { result = accumulator(result, element); }, [](const E &element) -> bool { return true; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	return result;
}

template <typename E>
template <typename R>
R Semantic<E>::reduce(const R &identity, const BiFunction<R, E, R> &accumulator) const
{
	R result = identity;
	(*generator)([&result, &accumulator](const E &element) -> void { result = accumulator(result, element); }, [](const E &element) -> bool { return true; }, [](const E &element, const Timestamp &index) -> Timestamp { return index; });
	return result;
}

template <typename E>
Semantic<E> Semantic<E>::reindex() const
{
	std::vector<E> v;
	(*generator)([&v](const E &element) -> void { v.push_back(element); }, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp {});
	return fromOrdered(v);
}

template <typename E>
Semantic<E> Semantic<E>::reverse() const
{
	return Semantic<E>(std::make_shared<Generator<E>>([this](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		(*generator)(accept, interrupt, [&redirect](const E &element, const Timestamp &index) -> Timestamp {
			return -redirect(element, index);
		});
	}));
}

template <typename E>
Semantic<E> Semantic<E>::shuffle() const
{
	return Semantic<E>(std::make_shared<Generator<E>>([this](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		(*generator)(accept, interrupt, [&redirect](const E &element, const Timestamp &index) -> Timestamp {
			return randomly<Timestamp>(-100, 100) * redirect(element, index);
		});
	}));
}

template <typename E>
Semantic<E> Semantic<E>::skip(const Module &n) const
{
	Module skipped = 0;
	auto newGenerator = std::make_shared<Generator<E>>([this, n, skipped](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) mutable {
		(*generator)([&](const E &element) -> void {
                if (skipped < n) {
                    skipped++;
                } else {
                    if (accept) accept(element);
                } },
					 [&](const E &element) -> bool {
						 return skipped >= n && (!interrupt || interrupt(element));
					 },
					 redirect);
	});
	return Semantic<E>(newGenerator);
}

template <typename E>
Semantic<E> Semantic<E>::sorted() const
{
	auto elements = toVector();
	std::sort(elements.begin(), elements.end());
	return fromOrdered(elements);
}

template <typename E>
Semantic<E> Semantic<E>::sorted(const Comparator<E, E> &comparator) const
{
	auto elements = toVector();
	std::sort(elements.begin(), elements.end(), [&comparator](const E &a, const E &b) {
		return comparator(a, b) < 0;
	});
	return fromOrdered(elements);
}

template <typename E>
Semantic<E> Semantic<E>::sub(const Module &start, const Module &end) const
{
	Timestamp minimum = std::max(start, end);
	Timestamp maximum = std::max(start, end);
	if (minimum == maximum)
	{
		return empty<E>();
	}
	return Semantic<E>(std::make_shared<Generator<E>>([this, minimum, maximum](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) -> void {
		Timestamp current = 0;
		(*generator)([&current, minimum](const E &element) -> void {
			current++;
			if(current >= minimum){
				action(element);
			} }, [&](const E &element) -> bool { return current >= maximum || interrupt(element); }, redirect);
	}));
}

template <typename E>
Semantic<E> Semantic<E>::takeWhile(const Predicate<E> &predicate) const
{
	return Semantic<E>(std::make_shared<Generator<E>>([this, predicate](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		bool taking = true;
		(*generator)([&](const E &element) -> void {
                if (taking && predicate(element) && accept) {
                    accept(element);
                }
                else {
                    taking = false;
                } }, [&](const E &element) -> bool { return taking && predicate(element) && (!interrupt || interrupt(element)); }, redirect);
	}));
}

template <typename E>
std::list<E> Semantic<E>::toList() const
{
	std::list<E> result;
	forEach([&result](const E &element) {
		result.push_back(element);
	});
	return result;
}

template <typename E>
template <typename K, typename V>
std::map<K, V> Semantic<E>::toMap(const Function<E, K> &keyExtractor, const Function<E, V> &valueExtractor) const
{
	std::map<K, V> result;
	forEach([&result, &keyExtractor, &valueExtractor](const E &element) {
		K key = keyExtractor(element);
		V value = valueExtractor(element);
		result[key] = value;
	});
	return result;
}

template <typename E>
std::set<E> Semantic<E>::toSet() const
{
	std::set<E> s;
	(*generator)([&s](const E &element) -> void { s.insert(element); }, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp {});
	return s;
}

template <typename E>
std::unordered_set<E> Semantic<E>::toUnorderedSet() const
{
	std::unordered_set<E> s;
	(*generator)([&s](const E &element) -> void { s.insert(element); }, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp {});
	return s;
}

template <typename E>
Statistics<E, E> Semantic<E>::toStatistics() const
{
	auto elements = toVector();
	return Statistics<E, E>(elements);
}

template <typename E>
template <typename R>
Statistics<E, R> Semantic<E>::toStatistics(const Function<E, R> &mapper) const
{
	auto elements = toVector();
	std::vector<R> mappedElements;
	for (const auto &element : elements)
	{
		mappedElements.push_back(mapper(element));
	}
	return Statistics<E, R>(mappedElements);
}

template <typename E>
std::vector<E> Semantic<E>::toVector() const
{
	std::vector<E> v;
	(*generator)([&v](const E &element) -> void { v.push_back(element); }, [](const E &element) -> bool { return false; }, [](const E &element, const Timestamp &index) -> Timestamp {});
	return fromOrdered(v);
}
//Semantic implementation ends here.

//Semantic factory creator starts here.
template <typename E>
Semantic<E> empty()
{
	return Semantic<E>(std::make_shared<Generator<E>>([](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		// Empty generator - no elements produced
	}));
}

template <typename E, typename... Args>
Semantic<E> of(Args &&... args)
{
	std::vector<E> elements = {E(std::forward<Args>(args))...};
	return Semantic<E>(std::make_shared<Generator<E>>([elements](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		for (Module i = 0; i < elements.size(); i++)
		{
			E element = elements[i];
			if (interrupt && interrupt(element))
			{
				break;
			}
			Timestamp index = i;
			if (redirect)
			{
				index = redirect(element, i);
			}
			if (accept)
				accept(element);
		}
	}));
}

template <typename E>
Semantic<E> fill(const E &element, const Module &count)
{
	return Semantic<E>(std::make_shared<Generator<E>>([element, count](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		for (Module i = 0; i < count; i++)
		{
			if (interrupt && interrupt(element))
			{
				break;
			}
			Timestamp index = i;
			if (redirect)
			{
				index = redirect(element, i);
			}
			if (accept)
				accept(element);
		}
	}));
}

template <typename E>
Semantic<E> fill(const Supplier<E> &supplier, const Module &count)
{
	return Semantic<E>(std::make_shared<Generator<E>>([supplier, count](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		for (Module i = 0; i < count; i++)
		{
			E element = supplier();
			if (interrupt && interrupt(element))
			{
				break;
			}
			Timestamp index = i;
			if (redirect)
			{
				index = redirect(element, i);
			}
			if (accept)
				accept(element);
		}
	}));
}

template <typename E>
Semantic<E> from(const E *array, const Module &length)
{
	if (length < 32768)
	{
		return fromOrdered(array, length);
	}
	return fromUnordered(array, length);
}

template <typename E>
Semantic<E> fromUnordered(const E *array, const Module &length)
{
	return Semantic<E>(std::make_shared<Generator<E>>([array, length](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		for (Module i = 0; i < length; i++)
		{
			E element = array[i];
			if (interrupt(element))
			{
				break;
			}
			accept(element);
		}
	}));
}

template <typename E>
Semantic<E> fromOrdered(const E *array, const Module &length)
{
	return Semantic<E>(std::make_shared<Generator<E>>([array, length](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		std::map<Timestamp, E> order;
		for (Module i = 0; i < length; i++)
		{
			E element = array[i];
			if (interrupt(element))
			{
				break;
			}
			Timestamp index = redirect(element, i);
			index = index <= 0 ? ((length - (std::abs(index) % length)) % length) : (index % length);
			order[index] = element;
		}

		for (const std::pair<Timestamp, E> &pair : order)
		{
			accept(pair.second);
		}
	}));
}

template <typename E, Module length>
Semantic<E> from(const std::array<E, length> &array)
{
	if(length < 32768){
		return fromOrdered<E, length>(array);
	}
	return fromUnordered<E, length>(array);
}

template <typename E, Module length>
Semantic<E> fromUnordered(const std::array<E, length> &array)
{
	return Semantic<E>(std::make_shared<Generator<E>>([array](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		for (const E& element : array)
		{
			if (interrupt(element))
			{
				break;
			}
			accept(element);
		}
	}));
}

template <typename E, Module length>
Semantic<E> fromOrdered(const std::array<E, length> &array)
{
	return Semantic<E>(std::make_shared<Generator<E>>([array](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		std::map<Timestamp, E> order;
		for (Module i = 0; i < array.size(); i++)
		{
			E element = array[i];
			if (interrupt(element))
			{
				break;
			}
			Timestamp index = redirect(element, i);
			index = index <= 0 ? ((length - (std::abs(index) % length)) % length) : (index % length);
			order[index] = element;
		}
		for (const std::pair<Timestamp, E> &pair : order)
		{
			accept(pair.second);
		}
	}));
}

template <typename E>
Semantic<E> from(const std::vector<E> &v)
{i
	if(v.size() < 32768){
		return fromOrdered(v);
	}
	return fromUnordered(v);
}

template <typename E>
Semantic<E> fromUnordered(const std::vector<E> &v)
{
	return Semantic<E>(std::make_shared<Generator<E>>([v](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		for (const E& element : v)
		{
			if (interrupt(element))
			{
				break;
			}
			accept(element);
		}
	}));
}

template <typename E>
Semantic<E> fromOrdered(const std::vector<E> &v)
{
	return Semantic<E>(std::make_shared<Generator<E>>([v](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		std::map<Timestamp, E> order;
		for (Module i = 0; i < v.size(); i++)
		{
			E element = v[i];
			if (interrupt(element))
			{
				break;
			}
			Timestamp index = redirect(element, i);
				index = index <= 0 ? ((length - (std::abs(index) % length)) % length) : (index % length);
			order[index] = element;
		}
		for (const std::pair<Timestamp, E> &pair : order)
		{
			accept(pair.second);
		}
	}));
}

template <typename E>
Semantic<E> from(const std::list<E> &l)
{
	if(l.size() < 32768){
		return fromOrdered(l);
	}
	return fromUnordered(l);
}

template <typename E>
Semantic<E> fromUnordered(const std::list<E> &l)
{
		return Semantic<E>(std::make_shared<Generator<E>>([l](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		for (const E& element : l)
		{
			if (interrupt(element))
			{
				break;
			}
			accept(element);
		}
	}));
}

template <typename E>
Semantic<E> fromOrdered(const std::list<E> &l)
{
	return Semantic<E>(std::make_shared<Generator<E>>([l](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		Module i = 0;
		for (const auto &element : l)
		{
			if (interrupt && interrupt(element))
			{
				break;
			}
			Timestamp index = i;
			if (redirect)
			{
				index = redirect(element, i);
			}
			if (accept)
				accept(element);
			i++;
		}
	}));
}

template <typename E>
Semantic<E> from(const std::set<E> &s)
{
	return Semantic<E>(std::make_shared<Generator<E>>([s](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		Module i = 0;
		for (const auto &element : s)
		{
			if (interrupt && interrupt(element))
			{
				break;
			}
			Timestamp index = i;
			if (redirect)
			{
				index = redirect(element, i);
			}
			if (accept)
				accept(element);
			i++;
		}
	}));
}

template <typename E>
Semantic<E> from(const std::unordered_set<E> &s)
{
	return Semantic<E>(std::make_shared<Generator<E>>([s](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		Module i = 0;
		for (const auto &element : s)
		{
			if (interrupt && interrupt(element))
			{
				break;
			}
			Timestamp index = i;
			if (redirect)
			{
				index = redirect(element, i);
			}
			if (accept)
				accept(element);
			i++;
		}
	}));
}

template <typename E>
Semantic<E> iterate(const Generator<E> &generator)
{
	return Semantic<E>(std::make_shared<Generator<E>>(generator));
}

template <typename E>
Semantic<E> range(const E &start, const E &end)
{
	return range(start, end, E(1));
}

template <typename E>
Semantic<E> range(const E &start, const E &end, const E &step)
{
	static_assert(std::is_arithmetic<E>::value, "Range requires arithmetic types");

	return Semantic<E>(std::make_shared<Generator<E>>([start, end, step](const Consumer<E> &accept, const Predicate<E> &interrupt, const BiFunction<E, Timestamp, Timestamp> &redirect) {
		E current = start;
		Timestamp index = 0;
		while ((step > E(0) && current < end) || (step < E(0) && current > end))
		{
			if (interrupt && interrupt(current))
			{
				break;
			}
			Timestamp currentIndex = index;
			if (redirect)
			{
				currentIndex = redirect(current, index);
			}
			if (accept)
				accept(current);
			current = current + step;
			index++;
		}
	}));
}
//Semantic factory creator ends here.
}; // namespace semantic

int main()
{
	int *array = new int[5]{5, 4, 3, 2, 1};
	auto s = semantic::fromOrdered(array, 5).redirect([](const int &element, auto index) -> auto {
		return index - 1;
	});
	s.cout();
	return 0;
}
