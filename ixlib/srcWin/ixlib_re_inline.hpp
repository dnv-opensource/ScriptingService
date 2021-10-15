// ----------------------------------------------------------------------------
//  Description      : Regular expressions string object
// ----------------------------------------------------------------------------
//  (c) Copyright 1998 by iXiONmedia, all rights reserved.
// ----------------------------------------------------------------------------

#ifndef IXLIB_RE_INLINE
#define IXLIB_RE_INLINE

#include <stack>

#define NUM_MIN(a, b) ((a) < (b) ? (a) : (b))

namespace ixion {

/**
A class implementing a generic regular expression matcher not only for strings.
If you are looking for a usual regular expresion parser, look at
ixion::regex_string.

If you query anything about the last match, and that last match did
never happen, behavior is undefined.
*/

template <class T>
class regex
{
public:
    // various helper classes -----------------------------------------------
    class backref_stack
    {
    private:
        struct backref_entry
        {
            enum
            {
                OPEN,
                CLOSE
            } Type;
            TIndex Index;
        };

        typedef std::vector<backref_entry> internal_stack;

        internal_stack Stack;

    public:
        typedef TSize rewind_info;

        void open(TIndex index)
        {
            backref_entry entry = {backref_entry::OPEN, index};
            Stack.push_back(entry);
        }

        void close(TIndex index)
        {
            backref_entry entry = {backref_entry::CLOSE, index};
            Stack.push_back(entry);
        }

        rewind_info getRewindInfo() const
        {
            return Stack.size();
        }

        void rewind(rewind_info ri)
        {
            Stack.erase(Stack.begin() + ri, Stack.end());
        }

        void clear()
        {
            Stack.clear();
        }

        TSize size()
        {
            TSize result = 0;

            FOREACH_CONST (first, Stack, internal_stack)
                if (first->Type == backref_entry::OPEN)
                    result++;

            return result;
        }

        T get(TIndex number, T const &candidate) const
        {
            TIndex level = 0, next_index = 0;
            TIndex start;
            TIndex startlevel;

            internal_stack::const_iterator first = Stack.begin(), last = Stack.end();
            while (first != last)
            {
                if (first->Type == backref_entry::OPEN)
                {
                    if (number == next_index)
                    {
                        start = first->Index;
                        startlevel = level;
                        level++;
                        break;
                    }
                    next_index++;
                    level++;
                }
                if (first->Type == backref_entry::CLOSE)
                    level--;
                first++;
            }

            if (first == last)
                EX_THROW(regex, ECRE_INVBACKREF)

            first++;

            while (first != last)
            {
                if (first->Type == backref_entry::OPEN)
                    level++;
                if (first->Type == backref_entry::CLOSE)
                {
                    level--;
                    if (startlevel == level)
                        return candidate.substr(start, first->Index - start);
                }
                first++;
            }
            EX_THROW(regex, ECRE_UNBALBACKREF)
        }
    };

    // matchers -------------------------------------------------------------
    class matcher
    {
    protected:
        matcher *Next;
        bool OwnNext;
        TSize MatchLength;

    public:
        matcher() : Next(NULL)
        {
        }

        virtual ~matcher()
        {
            if (Next && OwnNext)
                delete Next;
        }
        virtual matcher *duplicate() const = 0;

        TSize getMatchLength() const
        {
            return MatchLength;
        }

        TSize subsequentMatchLength() const
        {
            TSize totalml = 0;
            matcher const *object = this;
            while (object)
            {
                totalml += object->MatchLength;
                object = object->Next;
            }
            return totalml;
        }

        virtual TSize minimumMatchLength() const = 0;

        TSize minimumSubsequentMatchLength() const
        {
            TSize totalml = 0;
            matcher const *object = this;
            while (object)
            {
                totalml += object->minimumMatchLength();
                object = object->Next;
            }
            return totalml;
        }

        matcher *getNext() const
        {
            return Next;
        }

        virtual void setNext(matcher *next, bool ownnext = true)
        {
            Next = next;
            OwnNext = ownnext;
        }

        // this routine must set the MatchLength member correctly.
        virtual bool match(backref_stack &brstack, T const &candidate, TIndex at) = 0;

    protected:
        bool matchNext(backref_stack &brstack, T const &candidate, TIndex at) const
        {
            return Next ? Next->match(brstack, candidate, at) : true;
        }

        void copy(matcher const *src)
        {
            if (src->Next && src->OwnNext)
                setNext(src->Next->duplicate(), src->OwnNext);
            else
                setNext(NULL);
        }
    };

    class quantifier : public matcher
    {
    private:
        typedef matcher super;
        bool Greedy, MaxValid;
        TSize MinCount, MaxCount;
        matcher *Quantified;

        struct backtrack_stack_entry
        {
            TIndex Index;
            typename backref_stack::rewind_info RewindInfo;
        };

    public:
        quantifier() : Quantified(NULL)
        {
        }

        quantifier(bool greedy, TSize mincount) : Greedy(greedy), MaxValid(false), MinCount(mincount)
        {
        }

        quantifier(bool greedy, TSize mincount, TSize maxcount)
            : Greedy(greedy), MaxValid(true), MinCount(mincount), MaxCount(maxcount)
        {
        }

        ~quantifier()
        {
            if (Quantified)
                delete Quantified;
        }

        matcher *duplicate() const
        {
            quantifier *dupe = new quantifier();
            dupe->copy(this);
            return dupe;
        }

        TSize minimumMatchLength() const
        {
            if (Quantified)
                return MinCount * Quantified->minimumMatchLength();
            else
                return 0;
        }

        void setQuantified(matcher *quantified)
        {
            Quantified = quantified;
        }

        bool match(backref_stack &brstack, T const &candidate, TIndex at)
        {
            // this routine does speculative matching, so it must pay close attention
            // to rewind the backref stack appropriately.
            // NB: matchNext does the rewinding automatically, whereas speculative
            // matches of the quantified portion must be rewound.

            // There should be at least one character in each match, we'd
            // run to Baghdad otherwise.

            if (!Quantified)
                return matchNext(brstack, candidate, at);

            // calculate accurate maximum match count
            TSize quant_min = Quantified->minimumSubsequentMatchLength();
            if (quant_min == 0)
                quant_min = 1;

            TSize max_count = candidate.size() - at;
            if (Next)
                max_count -= Next->minimumSubsequentMatchLength();
            max_count = max_count / quant_min + 1;

            if (MaxValid)
                max_count = NUM_MIN(max_count, MaxCount);

            // check that at least MinCount matches take place (non-speculative)
            TIndex idx = at;
            for (TSize c = 1; c <= MinCount; c++)
                if (Quantified->match(brstack, candidate, idx))
                    idx += Quantified->subsequentMatchLength();
                else
                    return false;

            // determine number of remaining matches
            TSize remcount = max_count - MinCount;

            // test for the remaining matches in a way that depends on Greedy flag
            if (Greedy)
            {
                // try to gobble up as many matches of quantified part as possible
                // (speculative)

                std::stack<backtrack_stack_entry> successful_indices;
                {
                    backtrack_stack_entry entry = {idx, brstack.getRewindInfo()};
                    successful_indices.push(entry);
                }

                while (Quantified->match(brstack, candidate, idx) && successful_indices.size() - 1 < remcount)
                {
                    idx += Quantified->subsequentMatchLength();
                    backtrack_stack_entry entry = {idx, brstack.getRewindInfo()};
                    successful_indices.push(entry);
                }

                // backtrack until rest of sequence also matches
                while (successful_indices.size() && !matchNext(brstack, candidate, successful_indices.top().Index))
                {
                    brstack.rewind(successful_indices.top().RewindInfo);
                    successful_indices.pop();
                }

                if (successful_indices.size())
                {
                    MatchLength = successful_indices.top().Index - at;
                    return true;
                }
                else
                    return false;
            }
            else
            {
                for (TSize c = 0; c <= remcount; c++)
                {
                    if (matchNext(brstack, candidate, idx))
                    {
                        MatchLength = idx - at;
                        return true;
                    }
                    // following part runs once too much, effectively:
                    // if c == remcount, idx may be increased, but the search fails anyway
                    // => no problem
                    if (Quantified->match(brstack, candidate, idx))
                        idx += Quantified->subsequentMatchLength();
                    else
                        return false;
                }
                return false;
            }
        }

    protected:
        void copy(quantifier const *src)
        {
            super::copy(src);
            Greedy = src->Greedy;
            MaxValid = src->MaxValid;
            MinCount = src->MinCount;
            MaxCount = src->MaxCount;
            Quantified = src->Quantified->duplicate();
        }
    };

    class sequence_matcher : public matcher
    {
        T MatchStr;

    public:
        sequence_matcher(T const &matchstr) : MatchStr(matchstr)
        {
            MatchLength = MatchStr.size();
        }

        matcher *duplicate() const
        {
            sequence_matcher *dupe = new sequence_matcher(MatchStr);
            dupe->copy(this);
            return dupe;
        }

        TSize minimumMatchLength() const
        {
            return MatchStr.size();
        }

        bool match(backref_stack &brstack, T const &candidate, TIndex at)
        {
            if (at + MatchStr.size() > candidate.size())
                return false;
            return (T(candidate.begin() + at, candidate.begin() + at + MatchStr.size()) == MatchStr) &&
                   matchNext(brstack, candidate, at + MatchStr.size());
        }
    };

    class any_matcher : public matcher
    {
    public:
        any_matcher()
        {
            MatchLength = 1;
        }

        matcher *duplicate() const
        {
            any_matcher *dupe = new any_matcher();
            dupe->copy(this);
            return dupe;
        }

        TSize minimumMatchLength() const
        {
            return 1;
        }

        bool match(backref_stack &brstack, T const &candidate, TIndex at)
        {
            return at < candidate.size() && matchNext(brstack, candidate, at + 1);
        }
    };

    class start_matcher : public matcher
    {
    public:
        start_matcher()
        {
            MatchLength = 0;
        }

        matcher *duplicate() const
        {
            start_matcher *dupe = new start_matcher();
            dupe->copy(this);
            return dupe;
        }

        TSize minimumMatchLength() const
        {
            return 0;
        }

        bool match(backref_stack &brstack, T const &candidate, TIndex at)
        {
            return (at == 0) && matchNext(brstack, candidate, at);
        }
    };

    class end_matcher : public matcher
    {
    public:
        end_matcher()
        {
            MatchLength = 0;
        }

        matcher *duplicate() const
        {
            end_matcher *dupe = new end_matcher();
            dupe->copy(this);
            return dupe;
        }

        TSize minimumMatchLength() const
        {
            return 0;
        }

        bool match(backref_stack &brstack, T const &candidate, TIndex at)
        {
            return (at == candidate.size()) && matchNext(brstack, candidate, at);
        }
    };

    class backref_open_matcher : public matcher
    {
    public:
        backref_open_matcher()
        {
            MatchLength = 0;
        }

        matcher *duplicate() const
        {
            backref_open_matcher *dupe = new backref_open_matcher();
            dupe->copy(this);
            return dupe;
        }

        TSize minimumMatchLength() const
        {
            return 0;
        }

        bool match(backref_stack &brstack, T const &candidate, TIndex at)
        {
            backref_stack::rewind_info ri = brstack.getRewindInfo();
            brstack.open(at);

            bool result = matchNext(brstack, candidate, at);

            if (!result)
                brstack.rewind(ri);
            return result;
        }
    };

    class backref_close_matcher : public matcher
    {
    public:
        backref_close_matcher()
        {
            MatchLength = 0;
        }

        matcher *duplicate() const
        {
            backref_close_matcher *dupe = new backref_close_matcher();
            dupe->copy(this);
            return dupe;
        }

        TSize minimumMatchLength() const
        {
            return 0;
        }

        bool match(backref_stack &brstack, T const &candidate, TIndex at)
        {
            backref_stack::rewind_info ri = brstack.getRewindInfo();
            brstack.close(at);

            bool result = matchNext(brstack, candidate, at);

            if (!result)
                brstack.rewind(ri);
            return result;
        }
    };

    class alternative_matcher : public matcher
    {
        // The connector serves two purposes:
        // a) be a null-matcher that re-unites the different alternate token
        //    sequences
        // b) make the end of each sequence identifiable to be able to compute
        //    the match length

        class connector : public matcher
        {
        public:
            matcher *duplicate() const
            {
                return NULL;
            }

            TSize minimumMatchLength() const
            {
                return 0;
            }

            bool match(backref_stack &brstack, T const &candidate, TIndex at)
            {
                return matchNext(brstack, candidate, at);
            }
        };

        typedef matcher super;
        typedef std::vector<matcher *> alt_list;
        alt_list AltList;
        connector Connector;

    public:
        ~alternative_matcher()
        {
            while (AltList.size())
            {
                delete AltList.back();
                AltList.pop_back();
            }
        }

        matcher *duplicate() const
        {
            alternative_matcher *dupe = new alternative_matcher();
            dupe->copy(this);
            return dupe;
        }

        TSize minimumMatchLength() const
        {
            TSize result = 0;
            bool is_first = true;

            FOREACH_CONST (first, AltList, alt_list)
                if (is_first)
                {
                    result = (*first)->minimumMatchLength();
                    is_first = true;
                }
                else
                {
                    TSize current = (*first)->minimumMatchLength();
                    if (current < result)
                        result = current;
                }

            return result;
        }

        void setNext(matcher *next, bool ownnext = true)
        {
            matcher::setNext(next);
            Connector.setNext(next, false);
        }

        void addAlternative(matcher *alternative)
        {
            AltList.push_back(alternative);
            matcher *searchlast = alternative, *last = NULL;
            while (searchlast)
            {
                last = searchlast;
                searchlast = searchlast->getNext();
            }
            last->setNext(&Connector, false);
        }

        bool match(backref_stack &brstack, T const &candidate, TIndex at)
        {
            std::vector<matcher *>::iterator first = AltList.begin(), last = AltList.end();
            while (first != last)
            {
                if ((*first)->match(brstack, candidate, at))
                {
                    MatchLength = 0;
                    matcher const *object = *first;
                    while (object != &Connector)
                    {
                        MatchLength += object->getMatchLength();
                        object = object->getNext();
                    }
                    return true;
                }
                first++;
            }
            return false;
        }

    protected:
        void copy(alternative_matcher const *src)
        {
            super::copy(src);
            Connector.setNext(Next, false);

            FOREACH_CONST (first, src->AltList, alt_list)
                addAlternative((*first)->duplicate());
        }
    };

    class backref_matcher : public matcher
    {
        TIndex Backref;

    public:
        backref_matcher(TIndex backref) : Backref(backref)
        {
        }

        matcher *duplicate() const
        {
            backref_matcher *dupe = new backref_matcher(Backref);
            dupe->copy(this);
            return dupe;
        }

        TSize minimumMatchLength() const
        {
            return 0;
        }

        bool match(backref_stack &brstack, T const &candidate, TIndex at)
        {
            T matchstr = brstack.get(Backref, candidate);
            MatchLength = matchstr.size();

            if (at + matchstr.size() > candidate.size())
                return false;
            return (T(candidate.begin() + at, candidate.begin() + at + matchstr.size()) == matchstr) &&
                   matchNext(brstack, candidate, at + matchstr.size());
        }
    };

    // instance data --------------------------------------------------------
    std::auto_ptr<matcher> ParsedRegex;
    backref_stack BackrefStack;
    T LastCandidate;
    TIndex MatchIndex;
    TSize MatchLength;

public:
    // interface ------------------------------------------------------------
    regex();
    regex(regex const &src);

    regex &operator=(regex const &src);

    bool match(T const &candidate, TIndex from = 0);
    bool matchAt(T const &candidate, TIndex at = 0);

    // Queries pertaining to the last match
    TIndex getMatchIndex()
    {
        return MatchIndex;
    }

    TSize getMatchLength()
    {
        return MatchLength;
    }

    std::string getMatch()
    {
        return T(LastCandidate.begin() + MatchIndex, LastCandidate.begin() + MatchIndex + MatchLength);
    }

    TSize countBackrefs()
    {
        return BackrefStack.size();
    }

    T getBackref(TIndex index)
    {
        return BackrefStack.get(index, LastCandidate);
    }
};

/**
A regular expression parser and matcher.

Backref numbering starts at \0.

ReplaceAll does not set the MatchIndex/MatchGlobal members.

What is there is compatible with perl5. (See man perlre or
http://www.cpan.org/doc/manual/html/pod/perlre.html)
However, not everything is there. Here's what's missing:

<ul>
  <li> \Q-\E,\b,\B,\A,\Z,\z
  <li> discerning between line and string
  <li> (?#comments)
  <li> (?:clustering)
  <li> (?=positive lookahead assumptions)
  <li> (?!negative lookahead assumptions
  <li> (?<=positive lookbehind assumptions)
  <li> (?<!negative lookbehind assumptions
  <li> (?>independent substrings)
  <li> modifiers such as "case independent"
  </ul>

as well as all the stuff involving perl code, naturally.
None of these is actually hard to hack in. If you want them,
pester me or try for yourself (and submit patches!)
*/
class regex_string : public regex<std::string>
{
private:
    class class_matcher : public regex<std::string>::matcher
    {
    private:
        typedef regex<std::string>::matcher super;
#define CharValues 256
        bool Set[CharValues];
        bool Negated;

    public:
        class_matcher();
        class_matcher(std::string const &cls);

        matcher *duplicate() const;

        TSize minimumMatchLength() const
        {
            return 1;
        }

        bool match(backref_stack &brstack, std::string const &candidate, TIndex at);

    private:
        void expandClass(std::string const &cls);

    protected:
        void copy(class_matcher const *src);
    };

    class special_class_matcher : public regex<std::string>::matcher
    {
    public:
        enum type
        {
            DIGIT,
            NONDIGIT,
            ALNUM,
            NONALNUM,
            SPACE,
            NONSPACE
        };

    private:
        type Type;

    public:
        special_class_matcher(type tp);

        matcher *duplicate() const;

        TSize minimumMatchLength() const
        {
            return 1;
        }

        bool match(backref_stack &brstack, std::string const &candidate, TIndex at);
    };

public:
    regex_string()
    {
    }

    regex_string(std::string const &str)
    {
        parse(str);
    }

    regex_string(char const *s)
    {
        parse(s);
    }

    void parse(std::string const &expr);

    std::string replaceAll(std::string const &candidate, std::string const &replacement, TIndex from = 0);

private:
    regex<std::string>::matcher *parseRegex(std::string const &expr);
    quantifier *parseQuantifier(std::string const &expr, size_t &at);
    bool isGreedy(std::string const &expr, size_t &at);
};

}

#endif
