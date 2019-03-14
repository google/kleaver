#ifndef STRBUF_H
#define STRBUF_H

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/**
 * strbuf's are meant to be used with all the usual C string and memory
 * APIs. Given that the length of the buffer is known, it's often better to
 * use the mem* functions than a str* one (memchr vs. strchr e.g.).
 * Though, one has to be careful about the fact that str* functions often
 * stop on NULs and that strbufs may have embedded NULs.
 *
 * A strbuf is NUL terminated for convenience, but no function in the
 * strbuf API actually relies on the string being free of NULs.
 *
 * strbufs have some invariants that are very important to keep in mind:
 *
 *  - The `buf` member is never NULL, so it can be used in any usual C
 *    string operations safely. strbuf's _have_ to be initialized either by
 *    `strbuf_init()` or by `= STRBUF_INIT` before the invariants, though.
 *
 *    Do *not* assume anything on what `buf` really is (e.g. if it is
 *    allocated memory or not), use `strbuf_detach()` to unwrap a memory
 *    buffer from its strbuf shell in a safe way. That is the sole supported
 *    way. This will give you a malloced buffer that you can later `free()`.
 *
 *    However, it is totally safe to modify anything in the string pointed by
 *    the `buf` member, between the indices `0` and `len-1` (inclusive).
 *
 *  - The `buf` member is a byte array that has at least `len + 1` bytes
 *    allocated. The extra byte is used to store a `'\0'`, allowing the
 *    `buf` member to be a valid C-string. Every strbuf function ensure this
 *    invariant is preserved.
 *
 *    NOTE: It is OK to "play" with the buffer directly if you work it this
 *    way:
 *
 *        strbuf_grow(sb, SOME_SIZE); <1>
 *        strbuf_setlen(sb, sb->len + SOME_OTHER_SIZE);
 *
 *    <1> Here, the memory array starting at `sb->buf`, and of length
 *    `strbuf_avail(sb)` is all yours, and you can be sure that
 *    `strbuf_avail(sb)` is at least `SOME_SIZE`.
 *
 *    NOTE: `SOME_OTHER_SIZE` must be smaller or equal to `strbuf_avail(sb)`.
 *
 *    Doing so is safe, though if it has to be done in many places, adding the
 *    missing API to the strbuf module is the way to go.
 *
 *    WARNING: Do _not_ assume that the area that is yours is of size `alloc
 *    - 1` even if it's true in the current implementation. Alloc is somehow a
 *    "private" member that should not be messed with. Use `strbuf_avail()`
 *    instead.
*/

/**
 * Data Structures
 * ---------------
 */

/**
 * This is the string buffer structure. The `len` member can be used to
 * determine the current length of the string, and `buf` member provides
 * access to the string itself.
 */
struct strbuf {
	size_t alloc;
	size_t len;
	char *buf;
};

extern char strbuf_slopbuf[];
#define STRBUF_INIT  { .alloc = 0, .len = 0, .buf = strbuf_slopbuf }

/**
 * Life Cycle Functions
 * --------------------
 */

/**
 * Initialize the structure. The second parameter can be zero or a bigger
 * number to allocate memory, in case you want to prevent further reallocs.
 */
void strbuf_init(struct strbuf *sb, size_t alloc);

/**
 * Release a string buffer and the memory it used. After this call, the
 * strbuf points to an empty string that does not need to be free()ed, as
 * if it had been set to `STRBUF_INIT` and never modified.
 *
 * To clear a strbuf in preparation for further use without the overhead
 * of free()ing and malloc()ing again, use strbuf_reset() instead.
 */
void strbuf_release(struct strbuf *sb);

/**
 * Detach the string from the strbuf and returns it; you now own the
 * storage the string occupies and it is your responsibility from then on
 * to release it with `free(3)` when you are done with it.
 *
 * The strbuf that previously held the string is reset to `STRBUF_INIT` so
 * it can be reused after calling this function.
 */
char *strbuf_detach(struct strbuf *sb, size_t *sz);

/**
 * Attach a string to a buffer. You should specify the string to attach,
 * the current length of the string and the amount of allocated memory.
 * The amount must be larger than the string length, because the string you
 * pass is supposed to be a NUL-terminated string.  This string _must_ be
 * malloc()ed, and after attaching, the pointer cannot be relied upon
 * anymore, and neither be free()d directly.
 */
void strbuf_attach(struct strbuf *sb, void *str, size_t len, size_t mem);


/**
 * Functions related to the size of the buffer
 * -------------------------------------------
 */

/**
 * Determine the amount of allocated but unused memory.
 */
static inline size_t strbuf_avail(const struct strbuf *sb)
{
	return sb->alloc ? sb->alloc - sb->len - 1 : 0;
}

/**
 * Ensure that at least this amount of unused memory is available after
 * `len`. This is used when you know a typical size for what you will add
 * and want to avoid repetitive automatic resizing of the underlying buffer.
 * This is never a needed operation, but can be critical for performance in
 * some cases.
 */
void strbuf_grow(struct strbuf *sb, size_t amount);

/**
 * Set the length of the buffer to a given value. This function does *not*
 * allocate new memory, so you should not perform a `strbuf_setlen()` to a
 * length that is larger than `len + strbuf_avail()`. `strbuf_setlen()` is
 * just meant as a 'please fix invariants from this strbuf I just messed
 * with'.
 */
static inline void strbuf_setlen(struct strbuf *sb, size_t len)
{
	assert(len <= (sb->alloc ? sb->alloc - 1 : 0));
	sb->len = len;
	if (sb->buf != strbuf_slopbuf)
		sb->buf[len] = '\0';
	else
		assert(!strbuf_slopbuf[0]);
}

/**
 * Empty the buffer by setting the size of it to zero.
 */
#define strbuf_reset(sb)  strbuf_setlen(sb, 0)


/**
 * Functions related to the contents of the buffer
 * -----------------------------------------------
 */

/**
 * Strip whitespace from the beginning (`ltrim`), end (`rtrim`), or both side
 * (`trim`) of a string.
 */
void strbuf_trim(struct strbuf *sb);
void strbuf_rtrim(struct strbuf *sb);
void strbuf_ltrim(struct strbuf *sb);

/* Strip trailing LF or CR/LF */
void strbuf_trim_trailing_newline(struct strbuf *sb);

/**
 * Compare two buffers. Returns an integer less than, equal to, or greater
 * than zero if the first buffer is found, respectively, to be less than,
 * to match, or be greater than the second buffer.
 */
int strbuf_cmp(const struct strbuf *first, const struct strbuf *second);


/**
 * Adding data to the buffer
 * -------------------------
 *
 * NOTE: All of the functions in this section will grow the buffer as
 * necessary.  If they fail for some reason other than memory shortage and the
 * buffer hadn't been allocated before (i.e. the `struct strbuf` was set to
 * `STRBUF_INIT`), then they will free() it.
 */

/**
 * Add a single character to the buffer.
 */
static inline void strbuf_addch(struct strbuf *sb, int c)
{
	if (!strbuf_avail(sb))
		strbuf_grow(sb, 1);
	sb->buf[sb->len++] = c;
	sb->buf[sb->len] = '\0';
}

/**
 * Add a character the specified number of times to the buffer.
 */
void strbuf_addchars(struct strbuf *sb, int c, size_t n);

/**
 * Add data of given length to the buffer.
 */
void strbuf_add(struct strbuf *sb, const void *data, size_t len);

/**
 * Add a NUL-terminated string to the buffer.
 *
 * NOTE: This function will *always* be implemented as an inline or a macro
 * using strlen, meaning that this is efficient to write things like:
 *
 *     strbuf_addstr(sb, "immediate string");
 *
 */
static inline void strbuf_addstr(struct strbuf *sb, const char *s)
{
	strbuf_add(sb, s, strlen(s));
}

/**
 * Copy the contents of another buffer at the end of the current one.
 */
void strbuf_addbuf(struct strbuf *sb, const struct strbuf *sb2);

/**
 * Add a formatted string to the buffer.
 */
__attribute__((format (printf,2,3)))
void strbuf_addf(struct strbuf *sb, const char *fmt, ...);

__attribute__((format (printf,2,0)))
void strbuf_vaddf(struct strbuf *sb, const char *fmt, va_list ap);

/**
 * Read a given size of data from a FILE* pointer to the buffer.
 *
 * NOTE: The buffer is rewound if the read fails. If -1 is returned,
 * `errno` must be consulted, like you would do for `read(3)`.
 * `strbuf_read()`, `strbuf_read_file()` and `strbuf_getline_*()`
 * family of functions have the same behaviour as well.
 */
size_t strbuf_fread(struct strbuf *sb, size_t size, FILE *file);

/**
 * Write the whole content of the strbuf to the stream not stopping at
 * NUL bytes.
 */
ssize_t strbuf_write(struct strbuf *sb, FILE *stream);

/**
 * "Complete" the contents of `sb` by ensuring that either it ends with the
 * character `term`, or it is empty.  This can be used, for example,
 * to ensure that text ends with a newline, but without creating an empty
 * blank line if there is no content in the first place.
 */
static inline void strbuf_complete(struct strbuf *sb, char term)
{
	if (sb->len && sb->buf[sb->len - 1] != term)
		strbuf_addch(sb, term);
}

static inline void strbuf_complete_line(struct strbuf *sb)
{
	strbuf_complete(sb, '\n');
}

/**
 * Create a newly allocated string using printf format. You can do this easily
 * with a strbuf, but this provides a shortcut to save a few lines.
 */
__attribute__((format (printf, 1, 0)))
char *xstrvfmt(const char *fmt, va_list ap);
__attribute__((format (printf, 1, 2)))
char *xstrfmt(const char *fmt, ...);

#endif /* STRBUF_H */
