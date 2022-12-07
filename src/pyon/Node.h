//
// Created by pkua on 06.12.22.
//

#ifndef RAMPACK_NODE_H
#define RAMPACK_NODE_H

#include <memory>

#include "PyonException.h"


namespace pyon::ast {
    class BadNodeCastException : public ASTException {
        using ASTException::ASTException;
    };

    class Node : public std::enable_shared_from_this<Node> {
    public:
        enum Type {
            INT,
            FLOAT,
            BOOLEAN,
            STRING,
            NONE,
            ARRAY,
            DICTIONARY,
            DATACLASS
        };

    private:
        static std::string nameNodeType(Type type) {
            switch (type) {
                case INT:
                    return "INT";
                case FLOAT:
                    return "FLOAT";
                case BOOLEAN:
                    return "BOOLEAN";
                case STRING:
                    return "STRING";
                case NONE:
                    return "NONE";
                case ARRAY:
                    return "ARRAY";
                case DICTIONARY:
                    return "DICTIONARY";
                case DATACLASS:
                    return "DATACLASS";
                default:
                    throw std::runtime_error("Unknown node type");
            }
        }

    protected:
        Type nodeType{};
        explicit Node(Type nodeType) : nodeType{nodeType} { }

    public:
        virtual ~Node() = default;

        template <typename ConcreteNode>
        std::shared_ptr<const ConcreteNode> as() const
        {
            static_assert(std::is_base_of_v<Node, ConcreteNode>,
                          "Template parameter of pyon::ast::Node::as() should be derived from ConcreteNode");
            auto castPtr = std::dynamic_pointer_cast<const ConcreteNode>(this->shared_from_this());
            if (!castPtr) {
                auto expectedType = Node::nameNodeType(ConcreteNode::NODE_TYPE);
                auto actualType = Node::nameNodeType(castPtr->getType());
                throw BadNodeCastException("Trying to cast " + actualType + " node to " + expectedType);
            }
            return castPtr;
        }

        [[nodiscard]] Type getType() const { return this->nodeType; }
    };
} // ast

#endif //RAMPACK_NODE_H
