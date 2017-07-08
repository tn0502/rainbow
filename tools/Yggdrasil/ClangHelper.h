// Copyright (c) 2010-present Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#ifndef YGGDRASIL_CLANGHELPER_H_
#define YGGDRASIL_CLANGHELPER_H_

#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include <clang-c/Index.h>

class Spelling
{
public:
    Spelling(CXCursor cursor) : spelling_(clang_getCursorSpelling(cursor)) {}

    Spelling(CXCursorKind kind) : spelling_(clang_getCursorKindSpelling(kind))
    {
    }

    Spelling(CXType type) : spelling_(clang_getTypeSpelling(type)) {}
    Spelling(CXTypeKind kind) : spelling_(clang_getTypeKindSpelling(kind)) {}

    Spelling(Spelling&& sp) : spelling_(sp.spelling_)
    {
        sp.spelling_.data = nullptr;
    }

    ~Spelling()
    {
        if (spelling_.data == nullptr)
            return;

        clang_disposeString(spelling_);
    }

    auto c_str() const { return clang_getCString(spelling_); }

    bool operator==(const char* str) const { return strcmp(c_str(), str) == 0; }
    bool operator!=(const char* str) const { return !(*this == str); }

    friend auto operator<<(std::ostream& os, const Spelling& sp)
        -> std::ostream&
    {
        return os << sp.c_str();
    }

    Spelling(const Spelling&) = delete;
    Spelling& operator=(const Spelling&) = delete;

private:
    CXString spelling_;
};

class TranslationUnit
{
public:
    TranslationUnit() : index_(nullptr), translation_unit_(nullptr) {}

    TranslationUnit(const char* source_path,
                    const char* const* compiler_flags,
                    int num_flags)
        : index_(clang_createIndex(1, 1)),
          translation_unit_(
              clang_parseTranslationUnit(index_,
                                         source_path,
                                         compiler_flags,
                                         num_flags,
                                         nullptr,
                                         0,
                                         CXTranslationUnit_ForSerialization))
    {
    }

    TranslationUnit(TranslationUnit&& tu)
        : index_(tu.index_), translation_unit_(tu.translation_unit_)
    {
        tu.index_ = nullptr;
        tu.translation_unit_ = nullptr;
    }

    ~TranslationUnit()
    {
        if (translation_unit_ != nullptr)
            clang_disposeTranslationUnit(translation_unit_);
        if (index_ != nullptr)
            clang_disposeIndex(index_);
    }

    auto cursor() const
    {
        return clang_getTranslationUnitCursor(translation_unit_);
    }

    explicit operator bool() const { return translation_unit_ != nullptr; }

    TranslationUnit(const TranslationUnit&) = delete;
    auto operator=(const TranslationUnit&) -> TranslationUnit& = delete;

private:
    CXIndex index_;
    CXTranslationUnit translation_unit_;
};

template <typename T, size_t N>
constexpr auto array_size(const T (&)[N]) noexcept
{
    return N;
}

auto banner(std::ostream& os, const char* comment_start) -> std::ostream&
{
    os << comment_start << " This file was generated by Yggdrasil. Do not modify.\n"
       << comment_start << "\n"
       << comment_start << " Copyright (c) 2010-present Bifrost Entertainment AS and Tommy Nguyen\n"
       << comment_start << " Distributed under the MIT License.\n"
       << comment_start << " (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)\n"
       << "\n";
    return os;
}

auto name_of(CXType type) -> std::string
{
    bool should_trim = false;
    std::string name(Spelling(type).c_str());
    for (unsigned i = name.length() - 1; i > 0; --i)
    {
        const auto c = name[i];
        if (c == ' ')
        {
            if (should_trim)
                name.erase(name.begin() + i);
        }
        else
        {
            should_trim = c == '&' || c == '*';
        }
    }
    return name;
}

auto cursor_type(CXCursor cursor) -> std::string
{
    return name_of(clang_getCursorType(cursor));
}

auto decayed_cursor_type(CXCursor cursor) -> std::string
{
    auto type = cursor_type(cursor);
    if (auto last = type.cbegin() + type.length() - 1;
        *last == '&' || *last == '*')
    {
        type.erase(last);
    }

    constexpr char kConst[] = "const ";
    if (type.find_first_of(kConst) == 0)
        type.erase(0, array_size(kConst) - 1);

    return type;
}

void full_name(CXCursor cursor, std::string& name)
{
    if (clang_getCursorKind(cursor) == CXCursor_TranslationUnit)
        return;

    auto parent = clang_getCursorSemanticParent(cursor);
    full_name(parent, name);
    if (!name.empty())
        name += "::";
    name += Spelling(cursor).c_str();
}

auto full_name(CXCursor cursor) -> std::string
{
    std::string name;
    full_name(cursor, std::ref(name));
    return name;
}

bool is_designated_constructor(CXCursor cursor)
{
    auto parent = clang_getCursorLexicalParent(cursor);
    auto parent_kind = clang_getCursorKind(parent);
    return (parent_kind == CXCursor_StructDecl ||
            parent_kind == CXCursor_ClassDecl) &&
           clang_getCXXAccessSpecifier(cursor) == CX_CXXPublic &&
           !clang_Cursor_isVariadic(cursor) &&
           !clang_CXXConstructor_isMoveConstructor(cursor);
}

bool is_internal(CXCursor cursor)
{
    bool internal = false;
    clang_visitChildren(
        cursor,
        [](CXCursor cursor, CXCursor parent, CXClientData client_data) {
            switch (clang_getCursorKind(cursor))
            {
                case CXCursor_AnnotateAttr:
                    if (Spelling(cursor) == "rainbow internal")
                    {
                        *static_cast<bool*>(client_data) = true;
                        return CXChildVisit_Break;
                    }
                    break;
                default:
                    break;
            }
            return CXChildVisit_Continue;
        },
        &internal);
    return internal;
}

bool is_operator_overload(CXCursor cursor)
{
    return clang_getCursorKind(cursor) == CXCursor_CXXMethod &&
           std::string(Spelling(cursor).c_str()).find_first_of("operator") == 0;
}

bool is_public_class_method(CXCursor cursor, CXCursor parent)
{
    auto parent_kind = clang_getCursorKind(parent);
    return (parent_kind == CXCursor_StructDecl ||
            parent_kind == CXCursor_ClassDecl) &&
           clang_getCXXAccessSpecifier(cursor) == CX_CXXPublic &&
           !is_internal(cursor);
}

auto to_camelcase(const char* str) -> std::string
{
    std::string out;
    bool should_transform = true;
    const size_t length = strlen(str);
    for (size_t i = 0; i < length; ++i)
    {
        const auto c = str[i];
        if (c == '_')
        {
            should_transform = true;
            continue;
        }

        if (should_transform)
        {
            should_transform = false;
            out += std::toupper(c);
        }
        else
        {
            out += c;
        }
    }
    return out;
}

auto to_lower(const char* str) -> std::string
{
    std::string out(str);
    std::transform(out.begin(), out.end(), out.begin(), [](auto c) {
        return std::tolower(c);
    });
    return out;
}

template <typename T>
auto to_lower(const T& obj) -> std::string
{
    return to_lower(obj.c_str());
}

class ICursor
{
public:
    ICursor(CXCursor cursor) : cursor_(cursor), spelling_(cursor) {}

    ICursor(ICursor&& cursor)
        : cursor_(cursor.cursor_), spelling_(std::move(cursor.spelling_))
    {
    }

    auto cursor() const { return cursor_; }
    auto name() const { return spelling_.c_str(); }
    auto has_name() const { return name() != nullptr && name()[0] != '\0'; }

    auto root_namespace() const -> std::string
    {
        auto cursor = cursor_;
        auto parent = clang_getCursorSemanticParent(cursor);
        while (!clang_isTranslationUnit(clang_getCursorKind(parent)))
        {
            cursor = parent;
            parent = clang_getCursorSemanticParent(parent);
        }

        return Spelling(cursor).c_str();
    }

protected:
    ~ICursor() {}

private:
    CXCursor cursor_;
    Spelling spelling_;
};

class Argument : public ICursor
{
public:
    Argument(CXCursor cursor)
        : ICursor(cursor), type_(clang_getCursorType(cursor)),
          spelling_(cursor_type(cursor))
    {
    }

    auto kind() const { return type_.kind; }
    auto spelling() const { return spelling_.c_str(); }
    auto type() const { return type_; }

private:
    CXType type_;
    std::string spelling_;
};

class Result
{
public:
    Result(CXCursor cursor)
        : type_(clang_getCanonicalType(clang_getCursorResultType(cursor))),
          spelling_(name_of(clang_getCursorResultType(cursor))),
          name_(Spelling(cursor).c_str())
    {
        const auto start = name_.find_first_of('_');
        if (start != std::string::npos)
        {
            const auto end = name_.find_first_of('_', start + 1);
            if (end != std::string::npos)
                name_.erase(end);

            name_.erase(0, start + 1);
        }
    }

    auto name() const { return name_.c_str(); }
    auto spelling() const { return spelling_.c_str(); }
    auto type() const { return type_; }

private:
    CXType type_;
    std::string spelling_;
    std::string name_;
};

class Method : public ICursor
{
public:
    Method(CXCursor cursor, const ICursor& parent)
        : ICursor(cursor), result_(cursor)
    {
        entry_point_name_ = parent.root_namespace();
        entry_point_name_ += '_';
        entry_point_name_ += to_lower(parent.name());
        entry_point_name_ += '_';
        if (clang_getCursorKind(cursor) == CXCursor_Constructor)
            entry_point_name_ += "create";
        else
            entry_point_name_ += name();

        const auto num_args = clang_Cursor_getNumArguments(cursor);
        arguments_.reserve(num_args);
        for (int i = 0; i < num_args; ++i)
            arguments_.emplace_back(clang_Cursor_getArgument(cursor, i));
    }

    auto arguments() const -> const std::vector<Argument>&
    {
        return arguments_;
    }

    auto entry_point_name() const { return entry_point_name_.c_str(); }
    auto result() const -> const Result& { return result_; }

    auto infer_result_name() const -> std::string
    {
        std::string result = name();
        const auto start = result.find_first_of('_');
        if (start != std::string::npos)
        {
            const auto end = result.find_first_of('_', start + 1);
            if (end != std::string::npos)
                result.erase(end);

            result.erase(0, start + 1);
        }
        return result;
    }

    template <typename F>
    void join_arguments(std::ostream& os, F&& concat) const
    {
        if (arguments_.empty())
            return;

        auto i = arguments_.cbegin();
        concat(os, *i);
        for (++i; i != arguments_.cend(); ++i)
        {
            os << ", ";
            concat(os, *i);
        }
    }

    auto to_camelcase() const -> std::string
    {
        auto parent = clang_getCursorSemanticParent(cursor());
        std::string result = Spelling(parent).c_str();
        result += ::to_camelcase(name());
        return result;
    }

private:
    Result result_;
    std::string entry_point_name_;
    std::vector<Argument> arguments_;
};

class Class : public ICursor
{
public:
    Class(CXCursor cursor) : ICursor(cursor)
    {
        c_name_ = root_namespace();
        c_name_ += '_';
        c_name_ += to_lower(name());

        auto get_methods = [](CXCursor cursor,
                              CXCursor parent,
                              CXClientData client_data) -> CXChildVisitResult {
            const auto cursor_kind = clang_getCursorKind(cursor);
            if (cursor_kind == CXCursor_CXXMethod &&
                is_public_class_method(cursor, parent) &&
                !is_operator_overload(cursor))
            {
                auto self = static_cast<Class*>(client_data);
                self->methods_.emplace_back(cursor, *self);
            }
            else if (cursor_kind == CXCursor_Constructor &&
                     is_designated_constructor(cursor))
            {
                auto self = static_cast<Class*>(client_data);
                self->constructors_.emplace_back(cursor, *self);
            }

            return CXChildVisit_Continue;
        };
        clang_visitChildren(cursor, get_methods, this);
    }

    auto c_name() const { return c_name_.c_str(); }

    auto constructors() const -> const std::vector<Method>&
    {
        return constructors_;
    }

    auto methods() const -> const std::vector<Method>& { return methods_; }

private:
    std::string c_name_;
    std::vector<Method> constructors_;
    std::vector<Method> methods_;
};

struct IR
{
    const char* const* whitelist;
    size_t whitelist_size;
    std::vector<Class> models;
};

auto visit_children(CXCursor cursor, CXCursor parent, CXClientData client_data)
    -> CXChildVisitResult
{
    auto& data = *static_cast<IR*>(client_data);
    switch (clang_getCursorKind(cursor))
    {
        case CXCursor_StructDecl:
        case CXCursor_ClassDecl:
            if (auto name = full_name(cursor);
                std::any_of(data.whitelist,
                            data.whitelist + data.whitelist_size,
                            [&name](const char* c) { return name == c; }) &&
                clang_isCursorDefinition(cursor) &&
                std::none_of(std::cbegin(data.models),
                             std::cend(data.models),
                             [&name](const Class& model) {
                                 return name == full_name(model.cursor());
                             }))
            {
                data.models.emplace_back(cursor);
            }
            break;

        case CXCursor_Namespace:
            if (Spelling(cursor) != "std")  // Ignore `namespace std`.
                return CXChildVisit_Recurse;
            break;

        default:
            break;
    }

    return CXChildVisit_Continue;
}

#endif
