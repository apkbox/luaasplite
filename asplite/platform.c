
#if 0
#include <string.h>
#include <io.h>

//                   POSIX            Windows
//                   ---------------  ----------------------------------
// Fundamental type  char[]           wchar_t[]
// Encoding          unspecified*     UTF-16
// Separator         /                \, tolerant of /
// Drive letters     no               case-insensitive A-Z followed by :
// Alternate root    // (surprise!)   \\, for UNC paths
//
// * The encoding need not be specified on POSIX systems, although some
//   POSIX-compliant systems do specify an encoding.  Mac OS X uses UTF-8.
//   Chrome OS also uses UTF-8.
//   Linux does not specify an encoding, but in practice, the locale's
//   character set may be used.

// Null-terminated array of separators used to separate components in
// hierarchical paths.  Each character in this array is a valid separator,
// but kSeparators[0] is treated as the canonical separator and will be used
// when composing pathnames.
static const char kSeparators[] = "\\/";

// A special path component meaning "this directory."
static const char kCurrentDirectory[] = ".";

// A special path component meaning "the parent directory."
static const char kParentDirectory[] = "..";

// The character used to identify a file extension.
static const char kExtensionSeparator = ".";


char *NormalizePathSeparators(char *path)
{
#if _WIN32
    char *p = path;

    for (p = path; *p; p++) {
        char *s;
        for (s = &kSeparators[1]; *s; s++) {
            if (*p == *s) {
                *p = kSeparators[0];
                break;
            }
        }
    }
#else
    return path;
#endif
}

// If this FilePath contains a drive letter specification, returns the
// position of the last character of the drive letter specification,
// otherwise returns npos.  This can only be true on Windows, when a pathname
// begins with a letter followed by a colon.  On other platforms, this always
// returns npos.
int FindDriveLetter(const char *path)
{
#ifdef _WIN32
    // This is dependent on an ASCII-based character set, but that's a
    // reasonable assumption.  iswalpha can be too inclusive here.
    if (strlen(path) >= 2 && path[1] == ':' &&
            ((path[0] >= 'A' && path[0] <= 'Z') ||
            (path[0] >= 'a' && path[0] <= 'z'))) {
        return 1;
    }
#endif // _WIN32
    return -1;
}


#define array_size(a)   (sizeof(a) / sizeof((a)[0]))


int IsSeparator(const char *c)
{
    size_t i;

    for (i = 0; i < array_size(kSeparators) - 1; i++) {
        if (c == kSeparators[i]) {
            return 1;
        }
    }

    return 0;
}



char *StripTrailingSeparators(char *path)
{
    int pos;
    // If there is no drive letter, start will be 1, which will prevent
    // stripping the leading separator if there is only one separator.
    // If there is a drive letter, start will be set appropriately to prevent
    // stripping the first separator following the drive letter, if a separator
    // immediately follows the drive letter.
    int start = FindDriveLetter(path) + 2;
    int last_stripped = -1;

    for (pos = strlen(path);
            pos > start && IsSeparator(path[pos - 1]);
            --pos) {
        // If the string only has two separators and they're at the beginning,
        // don't strip them, unless the string began with more than two separators.
        if (pos != start + 1 || last_stripped == start + 2 ||
                !IsSeparator(path[start - 1])) {
            path[pos - 1] = 0;
            last_stripped = pos;
        }
    }

    return path;
}


// Returns a |dir_name| corresponding to the directory containing the path
// named by this object, stripping away the file component.  If this object
// only contains one component, returns a |dir_name| identifying
// kCurrentDirectory.  If this object already refers to the root directory,
// returns a FilePath identifying the root directory.
void DirName(const char *path, char *dir_name)
{
    char new_path[MAX_PATH];

    strcpy(new_path, path);
    StripTrailingSeparatorsInternal(new_path);

    // The drive letter, if any, always needs to remain in the output.  If there
    // is no drive letter, as will always be the case on platforms which do not
    // support drive letters, letter will be npos, or -1, so the comparisons and
    // resizes below using letter will still be valid.
    int letter = FindDriveLetter(new_path.path_);

    StringType::size_type last_separator =
        new_path.path_.find_last_of(kSeparators, StringType::npos,
        arraysize(kSeparators) - 1);
    if (last_separator == StringType::npos) {
        // path_ is in the current directory.
        new_path.path_.resize(letter + 1);
    }
    else if (last_separator == letter + 1) {
        // path_ is in the root directory.
        new_path.path_.resize(letter + 2);
    }
    else if (last_separator == letter + 2 &&
        IsSeparator(new_path.path_[letter + 1])) {
        // path_ is in "//" (possibly with a drive letter); leave the double
        // separator intact indicating alternate root.
        new_path.path_.resize(letter + 3);
    }
    else if (last_separator != 0) {
        // path_ is somewhere else, trim the basename.
        new_path.path_.resize(last_separator);
    }

    new_path.StripTrailingSeparatorsInternal();
    if (!new_path.path_.length())
        new_path.path_ = kCurrentDirectory;

    return new_path;
}

#endif
