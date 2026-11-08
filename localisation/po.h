#pragma once

// ===========================================================================
// localisation/po.h
//
// GNU gettext .po catalogue loading for Semantic-Cpp.
//
// The loader turns a .po stream into messages of a localisation::Catalog.  It
// deliberately uses only the public Catalog/Locale/PluralCategory API of
// localisation.h, so a .po file becomes the same kind of compiled ICU
// MessageFormat pattern that a hand-written catalogue uses.
//
// Example:
//
//   std::ifstream stream("locale/de/LC_MESSAGES/app.po");
//   localisation::Catalog catalog("en");
//   const std::size_t loaded =
//       localisation::loadPo(catalog, localisation::Locale("de"), stream);
//
// Supported .po subset
// --------------------
//   * msgctxt / msgid / msgid_plural / msgstr / msgstr[n] directives.
//   * Multi-line strings: a line that starts with '"' continues the value of
//     the previous directive.
//   * C escapes inside strings: \n \t \r \a \b \f \v \" \\ \' \?, octal
//     (\101) and hexadecimal (\x41) byte escapes.
//   * Comments: '#', '#.', '#:', '#|' and obsolete '#~' entries; obsolete
//     entries are ignored because they are comment lines.
//   * The '#,' flags line: the "fuzzy" flag is recorded on PoEntry and loadPo
//     skips fuzzy entries (gettext's runtime semantics for fuzzy messages are
//     intentionally not reproduced).
//   * The header entry (msgid "") and its Plural-Forms field:
//         Plural-Forms: nplurals=N; plural=EXPR;
//     EXPR is a small C expression over n supporting + - * / %, comparisons,
//     &&, ||, !, the ternary operator and parentheses.  It is evaluated for
//     n = 0..200 to learn which msgstr form belongs to which CLDR plural
//     category; the target Locale, not the header's Language field, decides
//     those categories.  Because gettext expressions are not written against
//     CLDR, several n values of one category can occasionally select
//     different forms; the most frequently sampled form wins and a tie keeps
//     the lowest index.
//   * Plural entries are compiled into one ICU MessageFormat string:
//         {count, plural, one {...} few {...} many {...} other {...}}
//     Callers must pass the number as the "count" argument, for example
//     catalog.translate(locale, id, Arguments().set("count", n)).
//     Missing categories are simply omitted and fall back to ICU's "other"
//     branch; "other" is always emitted (last form if the expression never
//     produces it in the sampled range).
//   * Without a Plural-Forms header the gettext default is used:
//     nplurals=2, plural=(n != 1).
//
// Notes and limitations
// ---------------------
//   * No charset conversion: input is read as UTF-8 regardless of the
//     Content-Type header.  A non-UTF-8 catalogue is loaded byte for byte.
//   * Fuzzy has no runtime semantics: fuzzy entries are skipped by loadPo and
//     merely flagged by parsePo.
//   * parsePo returns the header entry as well (its id is empty); loadPo never
//     writes it into the catalogue.
//   * Plural msgstr values are treated as plain text: '{', '}' and '\'' are
//     quoted for ICU, a literal '#' becomes a quoted '#' and the printf
//     integer conversions (%d, %i, %u, including positional %1$d and length
//     modifiers) become ICU's '#', which prints the count.  ICU arguments
//     such as {name} are therefore not interpreted inside plural forms.
//   * Singular msgstr values are inserted verbatim, so ICU arguments work.  If
//     a translation is not a valid ICU pattern it is quoted as literal text
//     instead of failing the load.
//   * This ICU dialect separates plural branches with whitespace only: a
//     comma between branches is a syntax error.  The generated pattern keeps
//     the single comma after the "plural" keyword but puts no commas between
//     branches.
//   * Malformed input throws std::invalid_argument carrying the line number:
//     an entry without msgstr, an invalid or duplicate msgstr index, an
//     unterminated string, an unknown keyword, or a second msgid without a
//     blank line.  loadPo propagates those errors.
//   * A malformed plural expression does not throw; it falls back to
//     nplurals=2, plural=(n != 1) so that one broken header field cannot make
//     a whole catalogue unloadable.
//   * Duplicate entries in a stream are written in order; the last one wins,
//     matching Catalog::add's insert_or_assign behaviour.
// ===========================================================================

#include "localisation.h"

#include <cstddef>
#include <istream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace localisation
{

// One logical .po entry.  forms holds the single msgstr value of a singular
// entry, or the msgstr[0], msgstr[1], ... values of a plural entry; a gap left
// by a missing index is stored as an empty string.
struct PoEntry
{
    std::string context;
    std::string id;
    std::vector<std::string> forms;
    bool fuzzy = false;
    // Extension to the four documented fields: true when the entry carried a
    // msgid_plural line.  Without it a plural entry in an nplurals=1
    // catalogue (zh, ja, ...) would be indistinguishable from a singular one,
    // because it has exactly one msgstr form.
    bool plural = false;
};

namespace detail::po
{
struct RawEntry
{
    PoEntry entry;
    std::size_t line = 0;
};

struct Document
{
    std::vector<RawEntry> entries;
};

struct PluralForms
{
    int nplurals = 2;
    std::string expression = "(n != 1)";
};

[[noreturn]] inline auto throwError(std::size_t line, const std::string &message) -> void
{
    throw std::invalid_argument("localisation/po.h:" + std::to_string(line) + ": " + message);
}

// Parses a UTF-8 .po stream.  The parser is a small state machine: a blank line
// ends an entry, a leading '"' continues the previous directive, a '#' line is
// a comment (only the flags line matters) and everything else is a directive.
class Parser
{
  public:
    explicit Parser(std::istream &input) : input_(input) {}

    auto parse() -> Document
    {
        std::string line;
        while (std::getline(input_, line))
        {
            ++line_;
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }
            if (firstLine_)
            {
                firstLine_ = false;
                if (line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
                {
                    line.erase(0, 3);
                }
            }
            handle(line);
        }
        if (input_.bad())
        {
            throwError(line_, "input stream failed while reading");
        }
        finalize();
        return std::move(document_);
    }

  private:
    enum class Target
    {
        none,
        context,
        id,
        plural,
        translation
    };

    void handle(std::string_view line)
    {
        std::size_t begin = 0;
        while (begin < line.size() && isSpace(line[begin]))
        {
            ++begin;
        }
        const std::string_view text = line.substr(begin);
        if (text.empty())
        {
            finalize();
            return;
        }
        if (text.front() == '#')
        {
            handleComment(text);
            return;
        }
        if (text.front() == '"')
        {
            if (target_ == Target::none)
            {
                throwError(line_, "string continuation without a preceding keyword");
            }
            appendStrings(text, 0);
            return;
        }

        std::size_t position = 0;
        while (position < text.size() &&
               (isAlpha(text[position]) || isDigit(text[position]) || text[position] == '_'))
        {
            ++position;
        }
        const std::string_view keyword = text.substr(0, position);
        if (keyword.empty())
        {
            throwError(line_, "expected a keyword");
        }
        beginEntry();

        if (keyword == "msgctxt")
        {
            if (hasContext_ || hasId_ || hasPlural_ || hasMsgstr_)
            {
                throwError(line_, "msgctxt must be the first field of an entry");
            }
            hasContext_ = true;
            context_.clear();
            target_ = Target::context;
        }
        else if (keyword == "msgid")
        {
            if (hasId_ || hasPlural_ || hasMsgstr_)
            {
                throwError(line_, "duplicate msgid (missing blank line between entries?)");
            }
            hasId_ = true;
            id_.clear();
            target_ = Target::id;
        }
        else if (keyword == "msgid_plural")
        {
            if (!hasId_)
            {
                throwError(line_, "msgid_plural before msgid");
            }
            if (hasPlural_ || hasMsgstr_)
            {
                throwError(line_, "duplicate msgid_plural");
            }
            hasPlural_ = true;
            plural_.clear();
            target_ = Target::plural;
        }
        else if (keyword == "msgstr")
        {
            if (!hasId_)
            {
                throwError(line_, "msgstr before msgid");
            }
            std::size_t index = 0;
            if (position < text.size() && text[position] == '[')
            {
                ++position;
                const std::size_t digits = position;
                std::size_t value = 0;
                while (position < text.size() && isDigit(text[position]))
                {
                    value = value * 10 + static_cast<std::size_t>(text[position] - '0');
                    if (value > 255)
                    {
                        throwError(line_, "msgstr index out of range (0-255)");
                    }
                    ++position;
                }
                if (digits == position || position >= text.size() || text[position] != ']')
                {
                    throwError(line_, "invalid msgstr index");
                }
                ++position;
                index = value;
            }
            if (index < formSeen_.size() && formSeen_[index])
            {
                throwError(line_, "duplicate msgstr[" + std::to_string(index) + "]");
            }
            if (formSeen_.size() <= index)
            {
                formSeen_.resize(index + 1, false);
                forms_.resize(index + 1);
            }
            formSeen_[index] = true;
            hasMsgstr_ = true;
            targetIndex_ = index;
            target_ = Target::translation;
        }
        else
        {
            throwError(line_, "unknown keyword '" + std::string(keyword) + "'");
        }

        appendStrings(text, position);
    }

    void handleComment(std::string_view text)
    {
        if (text.size() < 2 || text[0] != '#' || text[1] != ',')
        {
            return;
        }
        std::size_t start = 2;
        while (start <= text.size())
        {
            const std::size_t end = text.find(',', start);
            const std::string_view flag =
                trimView(text.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start));
            if (asciiLower(flag) == "fuzzy")
            {
                fuzzy_ = true;
            }
            if (end == std::string_view::npos)
            {
                break;
            }
            start = end + 1;
        }
    }

    void beginEntry()
    {
        if (open_)
        {
            return;
        }
        open_ = true;
        entryLine_ = line_;
    }

    void finalize()
    {
        if (!open_)
        {
            // Comments (including a pending "#, fuzzy") stay attached to the
            // next entry, so a blank line does not discard them.
            return;
        }
        if (!hasId_)
        {
            if (hasContext_)
            {
                throwError(entryLine_, "msgctxt without msgid");
            }
            reset();
            return;
        }
        if (!hasMsgstr_)
        {
            throwError(entryLine_, "entry has no msgstr");
        }

        RawEntry raw;
        raw.entry.context = context_;
        raw.entry.id = id_;
        raw.entry.forms = forms_;
        raw.entry.fuzzy = fuzzy_;
        raw.entry.plural = hasPlural_;
        raw.line = entryLine_;
        document_.entries.push_back(std::move(raw));
        reset();
    }

    void reset()
    {
        open_ = false;
        hasContext_ = false;
        hasId_ = false;
        hasPlural_ = false;
        hasMsgstr_ = false;
        fuzzy_ = false;
        context_.clear();
        id_.clear();
        plural_.clear();
        forms_.clear();
        formSeen_.clear();
        target_ = Target::none;
        targetIndex_ = 0;
    }

    auto targetText() -> std::string &
    {
        if (target_ == Target::translation)
        {
            if (targetIndex_ < forms_.size())
            {
                return forms_[targetIndex_];
            }
            throwError(line_, "msgstr slot is missing");
        }
        if (target_ == Target::context)
        {
            return context_;
        }
        if (target_ == Target::id)
        {
            return id_;
        }
        if (target_ == Target::plural)
        {
            return plural_;
        }
        throwError(line_, "string without a keyword");
    }

    void appendStrings(std::string_view text, std::size_t position)
    {
        std::size_t cursor = position;
        while (cursor < text.size() && isSpace(text[cursor]))
        {
            ++cursor;
        }
        if (cursor >= text.size() || text[cursor] != '"')
        {
            throwError(line_, "expected a quoted string");
        }
        std::string &sink = targetText();
        while (true)
        {
            parseQuotedString(text, cursor, sink, line_);
            while (cursor < text.size() && isSpace(text[cursor]))
            {
                ++cursor;
            }
            if (cursor >= text.size())
            {
                break;
            }
            if (text[cursor] != '"')
            {
                throwError(line_, "unexpected text after string");
            }
        }
    }

    static void parseQuotedString(std::string_view text, std::size_t &position, std::string &value, std::size_t line)
    {
        if (position >= text.size() || text[position] != '"')
        {
            throwError(line, "expected a quoted string");
        }
        ++position;
        while (position < text.size())
        {
            const char character = text[position];
            ++position;
            if (character == '"')
            {
                return;
            }
            if (character != '\\')
            {
                value.push_back(character);
                continue;
            }
            if (position >= text.size())
            {
                throwError(line, "unterminated escape sequence");
            }
            const char escape = text[position];
            ++position;
            switch (escape)
            {
            case 'n':
                value.push_back('\n');
                break;
            case 't':
                value.push_back('\t');
                break;
            case 'r':
                value.push_back('\r');
                break;
            case 'a':
                value.push_back('\a');
                break;
            case 'b':
                value.push_back('\b');
                break;
            case 'f':
                value.push_back('\f');
                break;
            case 'v':
                value.push_back('\v');
                break;
            case '"':
                value.push_back('"');
                break;
            case '\'':
                value.push_back('\'');
                break;
            case '?':
                value.push_back('?');
                break;
            case '\\':
                value.push_back('\\');
                break;
            case 'x':
            {
                int digits = 0;
                int byte = 0;
                while (digits < 2 && position < text.size())
                {
                    const int digit = hexValue(text[position]);
                    if (digit < 0)
                    {
                        break;
                    }
                    byte = byte * 16 + digit;
                    ++position;
                    ++digits;
                }
                if (digits == 0)
                {
                    throwError(line, "invalid \\x escape in string");
                }
                value.push_back(static_cast<char>(byte));
                break;
            }
            default:
                if (escape >= '0' && escape <= '7')
                {
                    int byte = escape - '0';
                    int digits = 1;
                    while (digits < 3 && position < text.size() && text[position] >= '0' && text[position] <= '7')
                    {
                        byte = byte * 8 + (text[position] - '0');
                        ++position;
                        ++digits;
                    }
                    value.push_back(static_cast<char>(byte & 0xFF));
                }
                else
                {
                    // Unknown escapes keep the escaped character, which is
                    // what gettext does for sequences such as '\ '.
                    value.push_back(escape);
                }
                break;
            }
        }
        throwError(line, "unterminated string");
    }

    std::istream &input_;
    std::size_t line_ = 0;
    bool firstLine_ = true;
    Document document_;

    bool open_ = false;
    bool hasContext_ = false;
    bool hasId_ = false;
    bool hasPlural_ = false;
    bool hasMsgstr_ = false;
    bool fuzzy_ = false;
    std::size_t entryLine_ = 0;

    std::string context_;
    std::string id_;
    std::string plural_;
    std::vector<std::string> forms_;
    std::vector<bool> formSeen_;

    Target target_ = Target::none;
    std::size_t targetIndex_ = 0;
};

inline auto parseDocument(std::istream &input) -> Document { return Parser(input).parse(); }

// Splits a "nplurals=N; plural=EXPR;" field into its two parts.  Anything that
// is not understood keeps the gettext default instead of failing the load.
inline auto parsePluralFormsValue(std::string_view value, PluralForms &forms) -> void
{
    std::size_t start = 0;
    while (start <= value.size())
    {
        const std::size_t end = value.find(';', start);
        const std::string_view item =
            trimView(value.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start));
        const std::size_t equals = item.find('=');
        if (equals != std::string_view::npos)
        {
            const std::string key = asciiLower(trimView(item.substr(0, equals)));
            const std::string_view raw = trimView(item.substr(equals + 1));
            if (key == "nplurals")
            {
                int count = 0;
                bool valid = !raw.empty();
                for (char character : raw)
                {
                    if (!isDigit(character))
                    {
                        valid = false;
                        break;
                    }
                    count = count * 10 + (character - '0');
                    if (count > 64)
                    {
                        valid = false;
                        break;
                    }
                }
                if (valid && count >= 1 && count <= 64)
                {
                    forms.nplurals = count;
                }
            }
            else if (key == "plural" && !raw.empty())
            {
                forms.expression = std::string(raw);
            }
        }
        if (end == std::string_view::npos)
        {
            break;
        }
        start = end + 1;
    }
}

inline auto parsePluralFormsHeader(std::string_view header) -> PluralForms
{
    PluralForms forms;
    std::size_t start = 0;
    while (start <= header.size())
    {
        const std::size_t end = header.find('\n', start);
        const std::string_view line =
            header.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start);
        const std::size_t colon = line.find(':');
        if (colon != std::string_view::npos)
        {
            const std::string key = asciiLower(trimView(line.substr(0, colon)));
            if (key == "plural-forms")
            {
                parsePluralFormsValue(trimView(line.substr(colon + 1)), forms);
            }
        }
        if (end == std::string_view::npos)
        {
            break;
        }
        start = end + 1;
    }
    return forms;
}

// The header entry is the first entry with an empty msgid; its msgstr[0] holds
// the "Field: value" lines of the file header.
inline auto pluralFormsOf(const Document &document) -> PluralForms
{
    for (const RawEntry &raw : document.entries)
    {
        if (!raw.entry.id.empty() || !raw.entry.context.empty() || raw.entry.forms.empty())
        {
            continue;
        }
        return parsePluralFormsHeader(raw.entry.forms.front());
    }
    return PluralForms{};
}

// A recursive-descent evaluator for the gettext plural expression language:
// integers, the operand n, + - * / %, comparisons, &&, ||, !, ?: and
// parentheses.  The reference expression is
//   (n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2)
// Division by zero yields 0 rather than undefined behaviour, because a broken
// expression must not take the whole process down while a catalogue loads.
class PluralExpression
{
  public:
    explicit PluralExpression(std::string_view text) : text_(text) {}

    auto evaluate(long long n) -> long long
    {
        n_ = n;
        position_ = 0;
        const long long value = parseConditional();
        skipSpaces();
        if (position_ != text_.size())
        {
            fail("trailing text");
        }
        return value;
    }

  private:
    auto atEnd() const -> bool { return position_ >= text_.size(); }

    auto peek() const -> char { return atEnd() ? '\0' : text_[position_]; }

    void skipSpaces()
    {
        while (!atEnd() && isSpace(text_[position_]))
        {
            ++position_;
        }
    }

    auto match(std::string_view token) -> bool
    {
        if (text_.compare(position_, token.size(), token) != 0)
        {
            return false;
        }
        position_ += token.size();
        return true;
    }

    [[noreturn]] void fail(const std::string &message) const
    {
        throw std::invalid_argument("localisation/po.h: " + message + " in plural expression '" +
                                    std::string(text_) + "'");
    }

    auto parseConditional() -> long long
    {
        const long long condition = parseLogicalOr();
        skipSpaces();
        if (peek() != '?')
        {
            return condition;
        }
        ++position_;
        const long long whenTrue = parseConditional();
        skipSpaces();
        if (peek() != ':')
        {
            fail("expected ':'");
        }
        ++position_;
        const long long whenFalse = parseConditional();
        return condition != 0 ? whenTrue : whenFalse;
    }

    auto parseLogicalOr() -> long long
    {
        long long value = parseLogicalAnd();
        while (true)
        {
            skipSpaces();
            if (!match("||"))
            {
                return value;
            }
            const long long right = parseLogicalAnd();
            value = (value != 0 || right != 0) ? 1 : 0;
        }
    }

    auto parseLogicalAnd() -> long long
    {
        long long value = parseEquality();
        while (true)
        {
            skipSpaces();
            if (!match("&&"))
            {
                return value;
            }
            const long long right = parseEquality();
            value = (value != 0 && right != 0) ? 1 : 0;
        }
    }

    auto parseEquality() -> long long
    {
        long long value = parseRelational();
        while (true)
        {
            skipSpaces();
            if (match("=="))
            {
                value = value == parseRelational() ? 1 : 0;
            }
            else if (match("!="))
            {
                value = value != parseRelational() ? 1 : 0;
            }
            else
            {
                return value;
            }
        }
    }

    auto parseRelational() -> long long
    {
        long long value = parseAdditive();
        while (true)
        {
            skipSpaces();
            if (match("<="))
            {
                value = value <= parseAdditive() ? 1 : 0;
            }
            else if (match(">="))
            {
                value = value >= parseAdditive() ? 1 : 0;
            }
            else if (match("<"))
            {
                value = value < parseAdditive() ? 1 : 0;
            }
            else if (match(">"))
            {
                value = value > parseAdditive() ? 1 : 0;
            }
            else
            {
                return value;
            }
        }
    }

    auto parseAdditive() -> long long
    {
        long long value = parseMultiplicative();
        while (true)
        {
            skipSpaces();
            if (match("+"))
            {
                value += parseMultiplicative();
            }
            else if (match("-"))
            {
                value -= parseMultiplicative();
            }
            else
            {
                return value;
            }
        }
    }

    auto parseMultiplicative() -> long long
    {
        long long value = parseUnary();
        while (true)
        {
            skipSpaces();
            if (match("*"))
            {
                value *= parseUnary();
            }
            else if (match("/"))
            {
                const long long divisor = parseUnary();
                value = divisor == 0 ? 0 : value / divisor;
            }
            else if (match("%"))
            {
                const long long divisor = parseUnary();
                value = divisor == 0 ? 0 : value % divisor;
            }
            else
            {
                return value;
            }
        }
    }

    auto parseUnary() -> long long
    {
        skipSpaces();
        if (match("!"))
        {
            return parseUnary() == 0 ? 1 : 0;
        }
        if (match("-"))
        {
            return -parseUnary();
        }
        if (match("+"))
        {
            return parseUnary();
        }
        return parsePrimary();
    }

    auto parsePrimary() -> long long
    {
        skipSpaces();
        if (match("("))
        {
            const long long value = parseConditional();
            skipSpaces();
            if (!match(")"))
            {
                fail("expected ')'");
            }
            return value;
        }
        if (isDigit(peek()))
        {
            long long value = 0;
            while (!atEnd() && isDigit(peek()))
            {
                value = value * 10 + (peek() - '0');
                if (value > 1000000000LL)
                {
                    fail("integer literal out of range");
                }
                ++position_;
            }
            return value;
        }
        if (peek() == 'n' || peek() == 'N')
        {
            ++position_;
            return n_;
        }
        fail("expected a number, 'n' or '('");
    }

    std::string_view text_;
    std::size_t position_ = 0;
    long long n_ = 0;
};

// Samples n = 0..200, maps every sampled value to its CLDR category for the
// target locale and records which msgstr index the plural expression selects.
// Returns false when the expression cannot be evaluated at all.
inline auto tallyPluralForms(const Locale &locale, int nplurals, const std::string &expression,
                             const std::vector<std::string> &forms,
                             std::map<PluralCategory, std::map<std::size_t, std::size_t>> &tally) -> bool
{
    if (nplurals < 1 || expression.empty())
    {
        return false;
    }
    PluralExpression evaluator(expression);
    try
    {
        (void)evaluator.evaluate(0);
    }
    catch (const std::invalid_argument &)
    {
        return false;
    }

    for (int n = 0; n <= 200; ++n)
    {
        long long index = 0;
        try
        {
            index = evaluator.evaluate(n);
        }
        catch (const std::invalid_argument &)
        {
            return false;
        }
        if (index < 0)
        {
            index = 0;
        }
        if (index >= static_cast<long long>(nplurals))
        {
            index = static_cast<long long>(nplurals) - 1;
        }
        if (static_cast<std::size_t>(index) >= forms.size())
        {
            continue;
        }
        const std::size_t form = static_cast<std::size_t>(index);
        if (forms[form].empty())
        {
            continue;
        }
        ++tally[pluralCategory(locale, n)][form];
    }
    return true;
}

// Chooses one msgstr index per CLDR category.  The most frequently sampled
// index wins; ties keep the lowest index.  "other" is always present so the
// generated ICU pattern is valid and unmapped categories can fall back to it.
inline auto planPluralBranches(const Locale &locale, int nplurals, const std::string &expression,
                               const std::vector<std::string> &forms)
    -> std::map<PluralCategory, std::size_t>
{
    std::map<PluralCategory, std::map<std::size_t, std::size_t>> tally;
    if (forms.empty() || !tallyPluralForms(locale, nplurals, expression, forms, tally))
    {
        tally.clear();
        if (!tallyPluralForms(locale, 2, "(n != 1)", forms, tally))
        {
            tally.clear();
        }
    }

    std::map<PluralCategory, std::size_t> plan;
    for (const auto &category : tally)
    {
        std::size_t best = 0;
        std::size_t bestCount = 0;
        for (const auto &candidate : category.second)
        {
            if (candidate.second > bestCount)
            {
                best = candidate.first;
                bestCount = candidate.second;
            }
        }
        plan[category.first] = best;
    }

    if (plan.find(PluralCategory::other) == plan.end() && !forms.empty())
    {
        std::size_t fallback = forms.size() - 1;
        while (fallback > 0 && forms[fallback].empty())
        {
            --fallback;
        }
        plan[PluralCategory::other] = fallback;
    }
    return plan;
}

// Returns the position just past a printf integer conversion starting at
// 'position' (which must point at '%'), or npos when the conversion is not an
// integer one.  %d, %i and %u are the placeholders gettext plural forms use
// for the count; %s and floating point conversions are left untouched.
inline auto integerPlaceholderEnd(std::string_view text, std::size_t position) -> std::size_t
{
    std::size_t cursor = position + 1;
    const std::size_t digits = cursor;
    while (cursor < text.size() && isDigit(text[cursor]))
    {
        ++cursor;
    }
    if (cursor > digits && cursor < text.size() && text[cursor] == '$')
    {
        ++cursor;
    }
    else
    {
        cursor = position + 1;
    }
    while (cursor < text.size() && (text[cursor] == '-' || text[cursor] == '+' || text[cursor] == ' ' ||
                                    text[cursor] == '0' || text[cursor] == '#'))
    {
        ++cursor;
    }
    while (cursor < text.size() && isDigit(text[cursor]))
    {
        ++cursor;
    }
    if (cursor < text.size() && text[cursor] == '.')
    {
        ++cursor;
        while (cursor < text.size() && isDigit(text[cursor]))
        {
            ++cursor;
        }
    }
    while (cursor < text.size() && (text[cursor] == 'h' || text[cursor] == 'l' || text[cursor] == 'j' ||
                                    text[cursor] == 'z' || text[cursor] == 't' || text[cursor] == 'L'))
    {
        ++cursor;
    }
    if (cursor < text.size() && (text[cursor] == 'd' || text[cursor] == 'i' || text[cursor] == 'u'))
    {
        return cursor + 1;
    }
    return std::string_view::npos;
}

// Escapes one plural msgstr value as literal ICU text: braces and apostrophes
// are quoted, a literal '#' becomes '#' and %% becomes %.  printf integer
// conversions are replaced with ICU's '#' (the count of the enclosing plural).
inline auto pluralBranchText(std::string_view text) -> std::string
{
    std::string result;
    result.reserve(text.size() + 8);
    std::size_t position = 0;
    while (position < text.size())
    {
        const char character = text[position];
        if (character == '%')
        {
            if (position + 1 < text.size() && text[position + 1] == '%')
            {
                result.push_back('%');
                position += 2;
                continue;
            }
            const std::size_t end = integerPlaceholderEnd(text, position);
            if (end != std::string_view::npos)
            {
                result.push_back('#');
                position = end;
                continue;
            }
            result.push_back('%');
            ++position;
            continue;
        }
        if (character == '\'')
        {
            result += "''";
        }
        else if (character == '{')
        {
            result += "'{'";
        }
        else if (character == '}')
        {
            result += "'}'";
        }
        else if (character == '#')
        {
            result += "'#'";
        }
        else
        {
            result.push_back(character);
        }
        ++position;
    }
    return result;
}

// Quotes a whole message as literal ICU text, used when a singular msgstr is
// not a valid ICU pattern by itself.
inline auto quoteIcuText(std::string_view text) -> std::string
{
    std::string result;
    result.reserve(text.size() + 8);
    for (char character : text)
    {
        if (character == '\'')
        {
            result += "''";
        }
        else if (character == '{')
        {
            result += "'{'";
        }
        else if (character == '}')
        {
            result += "'}'";
        }
        else
        {
            result.push_back(character);
        }
    }
    return result;
}

inline auto buildPluralPattern(const std::vector<std::string> &forms,
                               const std::map<PluralCategory, std::size_t> &plan) -> std::string
{
    static const PluralCategory order[] = {PluralCategory::zero, PluralCategory::one, PluralCategory::two,
                                           PluralCategory::few,  PluralCategory::many, PluralCategory::other};

    std::string pattern = "{count, plural,";
    for (PluralCategory category : order)
    {
        const auto found = plan.find(category);
        if (found == plan.end())
        {
            continue;
        }
        const std::size_t index = found->second;
        if (index >= forms.size())
        {
            continue;
        }
        // Branches are separated by spaces: this ICU parser does not accept a
        // comma between plural branches.
        pattern += ' ';
        pattern += pluralCategoryName(category);
        pattern += " {";
        pattern += pluralBranchText(forms[index]);
        pattern += '}';
    }
    pattern += '}';
    return pattern;
}

inline void addMessage(Catalog &catalog, const Locale &locale, const std::string &context, const std::string &id,
                       const std::string &pattern)
{
    if (context.empty())
    {
        catalog.add(locale, id, pattern);
    }
    else
    {
        catalog.add(locale, context, id, pattern);
    }
}
} // namespace detail::po

// Parses a .po stream into entries.  The header entry (empty msgid) is part of
// the result; plural entries also expose PoEntry::plural.  Malformed input
// throws std::invalid_argument with the offending line number.
inline auto parsePo(std::istream &input) -> std::vector<PoEntry>
{
    const detail::po::Document document = detail::po::parseDocument(input);
    std::vector<PoEntry> entries;
    entries.reserve(document.entries.size());
    for (const detail::po::RawEntry &raw : document.entries)
    {
        entries.push_back(raw.entry);
    }
    return entries;
}

// Loads a .po stream into catalog under locale and returns the number of
// messages written.  The header, fuzzy entries and entries without any
// translation are skipped.  Singular entries are inserted verbatim; plural
// entries are compiled into one ICU plural pattern using the CLDR plural
// categories of locale.
inline auto loadPo(Catalog &catalog, const Locale &locale, std::istream &input) -> std::size_t
{
    const detail::po::Document document = detail::po::parseDocument(input);
    const detail::po::PluralForms header = detail::po::pluralFormsOf(document);

    std::size_t loaded = 0;
    for (const detail::po::RawEntry &raw : document.entries)
    {
        const PoEntry &entry = raw.entry;
        if (entry.id.empty())
        {
            // The header entry, or an entry without a usable identifier.
            continue;
        }
        if (entry.fuzzy)
        {
            continue;
        }

        bool translated = false;
        for (const std::string &form : entry.forms)
        {
            if (!form.empty())
            {
                translated = true;
                break;
            }
        }
        if (!translated)
        {
            continue;
        }

        if (entry.plural || entry.forms.size() > 1)
        {
            const std::map<PluralCategory, std::size_t> plan = detail::po::planPluralBranches(
                locale, header.nplurals, header.expression, entry.forms);
            detail::po::addMessage(catalog, locale, entry.context, entry.id,
                                   detail::po::buildPluralPattern(entry.forms, plan));
        }
        else
        {
            try
            {
                detail::po::addMessage(catalog, locale, entry.context, entry.id, entry.forms.front());
            }
            catch (const std::invalid_argument &)
            {
                // Not a valid ICU pattern: keep the translation as literal text
                // instead of rejecting the whole catalogue.
                detail::po::addMessage(catalog, locale, entry.context, entry.id,
                                       detail::po::quoteIcuText(entry.forms.front()));
            }
        }
        ++loaded;
    }
    return loaded;
}

} // namespace localisation
