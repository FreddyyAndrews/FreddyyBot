#ifndef CHAR_UTILS
#define CHAR_UTILS

inline bool is_white_piece(char piece)
{
    return piece >= 'A' && piece <= 'Z';
}

inline bool is_black_piece(char piece)
{
    return piece >= 'a' && piece <= 'z';
}

inline char to_lower(char c)
{
    unsigned char uc = static_cast<unsigned char>(c);
    if (uc >= 'A' && uc <= 'Z')
    {
        return static_cast<char>(uc + ('a' - 'A'));
    }
    return c;
}

#endif // CHAR_UTILS
