//
// Created by Piotr Kubala on 12/12/2022.
//

#ifndef RAMPACK_MATCHERDATACLASS_H
#define RAMPACK_MATCHERDATACLASS_H

#include <utility>
#include <vector>
#include <optional>

#include "MatcherBase.h"
#include "MatcherArray.h"
#include "MatcherDictionary.h"
#include "NodeDataclass.h"


namespace pyon::matcher {
    class DataclassException : public MatchException {
        using MatchException::MatchException;
    };

    class NoSuchArgumentException : public DataclassException {
        using DataclassException::DataclassException;
    };

    struct StandardArgument {
        std::string name;
        Any value;

        StandardArgument() = default;
        StandardArgument(std::string name, Any value) : name{std::move(name)}, value{std::move(value)} { }
    };

    class StandardArguments {
    private:
        std::vector<StandardArgument> arguments;

    public:
        using iterator = decltype(arguments)::const_iterator;

        StandardArguments() = default;
        explicit StandardArguments(std::vector<StandardArgument> arguments);

        [[nodiscard]] bool empty() const { return this->arguments.empty(); }
        [[nodiscard]] std::size_t size() const { return this->arguments.size(); }
        [[nodiscard]] const StandardArgument &front() const;
        [[nodiscard]] const StandardArgument &back() const;
        [[nodiscard]] iterator begin() const { return this->arguments.begin(); }
        [[nodiscard]] iterator end() const { return this->arguments.end(); }
        [[nodiscard]] const StandardArgument &at(std::size_t idx) const;
        [[nodiscard]] const Any &at(const std::string &name) const;
        [[nodiscard]] const StandardArgument &operator[](std::size_t idx) const { return this->at(idx); };
        [[nodiscard]] const Any &operator[](const std::string &name) const { return this->at(name); };
        [[nodiscard]] bool hasArgument(const std::string &name) const;
    };

    class DataclassData {
    private:
        StandardArguments standardArguments;
        ArrayData variadicArguments;
        DictionaryData variadicKeywordArguments;

    public:
        DataclassData(StandardArguments standardArguments, ArrayData variadicArguments,
                      DictionaryData variadicKeywordArguments)
                : standardArguments{std::move(standardArguments)}, variadicArguments{std::move(variadicArguments)},
                  variadicKeywordArguments{std::move(variadicKeywordArguments)}
        { }

        [[nodiscard]] const StandardArguments &getStandardArguments() const { return this->standardArguments; }
        [[nodiscard]] const ArrayData &getVariadicArguments() const { return this->variadicArguments; }
        [[nodiscard]] const DictionaryData &getVariadicKeywordArguments() const {
            return this->variadicKeywordArguments;
        }

        [[nodiscard]] std::size_t size() const;
        [[nodiscard]] std::size_t positionalSize() const;
        [[nodiscard]] bool empty() const;
        [[nodiscard]] bool positionalEmpty() const;
        [[nodiscard]] const Any &at(std::size_t idx) const;
        [[nodiscard]] const Any &at(const std::string &name) const;
        [[nodiscard]] const Any &operator[](std::size_t idx) const { return this->at(idx); };
        [[nodiscard]] const Any &operator[](const std::string &name) const { return this->at(name); };
    };

    class StandardArgumentSpecification {
    private:
        std::string name;
        std::shared_ptr<MatcherBase> matcher;
        std::optional<Any> defaultValue;

    public:
        StandardArgumentSpecification(std::string name) : name{std::move(name)} {}
        StandardArgumentSpecification(const char *const name) : name{name} {}

        template<typename ConcreteMatcher, typename = std::enable_if_t<std::is_base_of_v<MatcherBase, ConcreteMatcher>>>
        StandardArgumentSpecification(std::string name, const ConcreteMatcher &matcher)
                : name{std::move(name)}, matcher{std::make_shared<ConcreteMatcher>(matcher)}
        { }

        template<typename ConcreteMatcher, typename = std::enable_if_t<std::is_base_of_v<MatcherBase, ConcreteMatcher>>>
        StandardArgumentSpecification(std::string name, const ConcreteMatcher &matcher, const Any &defaultValue)
                : name{std::move(name)}, matcher{std::make_shared<ConcreteMatcher>(matcher)},
                  defaultValue{defaultValue}
        { }

        [[nodiscard]] const std::string &getName() const { return this->name; }
        [[nodiscard]] const std::shared_ptr<MatcherBase> &getMatcher() const { return this->matcher; }
        [[nodiscard]] bool hasMatcher() const { return this->matcher != nullptr;}
        [[nodiscard]] const std::optional<Any> &getDefaultValue() const { return this->defaultValue; }
        [[nodiscard]] bool hasDefaultValue() const { return this->defaultValue != std::nullopt; }
    };

    class MatcherDataclass : public MatcherBase {
    private:
        std::string name;
        std::vector<StandardArgumentSpecification> argumentsSpecification;
        MatcherArray variadicArgumentsMatcher = MatcherArray{}.empty();
        MatcherDictionary variadicKeywordArgumentsMatcher = MatcherDictionary{}.empty();
        std::vector<std::function<bool(const DataclassData&)>> filters;
        std::function<Any(const DataclassData&)> mapping = [](const DataclassData &dataclass) { return dataclass; };

        bool matchStandardArguments(StandardArguments &arguments,
                                    const std::shared_ptr<const ast::NodeArray> &nodePositional,
                                    const std::shared_ptr<const ast::NodeDictionary> &nodeKeyword) const;

        bool matchVariadicArguments(ArrayData &arguments,
                                    const std::shared_ptr<const ast::NodeArray> &nodePositional) const;

        bool matchKeywordVariadicArguments(DictionaryData &arguments,
                                           const std::shared_ptr<const ast::NodeDictionary> &nodeKeyword) const;

        bool emplaceArgument(std::vector<StandardArgument> &standardArgumentsVec,
                             const StandardArgumentSpecification &argumentSpecification,
                             const std::shared_ptr<const ast::Node> &argumentNode) const;

    public:
        explicit MatcherDataclass(std::string className);
        MatcherDataclass(std::string className, std::vector<StandardArgumentSpecification> argumentsSpecification);

        MatcherDataclass &arguments(std::vector<StandardArgumentSpecification> argumentsSpecification_);
        MatcherDataclass &variadicArguments(const MatcherArray &variadicMatcher = MatcherArray{});
        MatcherDataclass &variadicKeywordArguments(const MatcherDictionary &variadicMatcher = MatcherDictionary{});

        bool match(std::shared_ptr<const ast::Node> node, Any &result) const override;

        MatcherDataclass &mapTo(const std::function<Any(const DataclassData&)> &mapping_);
        MatcherDataclass &filter(const std::function<bool(const DataclassData&)> &filter);
    };
} // matcher

#endif //RAMPACK_MATCHERDATACLASS_H
