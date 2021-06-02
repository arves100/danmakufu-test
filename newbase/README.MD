# NewBase

Experimental, mostly broken, port of danmakufu to linux

Do not expect it to work, it might never do.

## Things
- Full BGFX support, ditch Direct3D9 ENTIRELY
- Modern C++ were applicable, no more gstd::shared_ptr clones
- Keep the Dx library just a render library, no reason to hold window initialization there
- Avoid shared pointers were possible
- Try to keep the code similar to the original one (in terms of API)
- No more widechars, see notes for more informations
- Avoid using Native API as much as possible, in case it's not possible avoid them create an API that doesn't depend on them (like avoid using Windows typenames for return types) so it would be easier to convert them.

## Other documents
- User's conversion guide (for game makers)
- Developer's conversion guide (for engine developers)

## Wide string notes
As you probably know, 99% of the operative systems out there uses UTF-8 strings, something
that Windows can't really follow (they started now with Windows10 1909 I think), which is why
MS inctroduced to us the Unicode API, which is actually UCS-2 not UTF-16 (again, against standards).
While for Dhn, I had two main ideas, I could store everything in widechar (2byte per character) and let all
the native linux apis converted to utf8, or touch all the code and store everything in multibyte/char (1byte per character).

Under Linux, it only supports UTF-8, keeping Widechar would cause I regression everytime a native api is used, as you would
have to convert it (for example with libicu) to a multibyte utf8 string before using it.

SDL2 and BGFX does not support Widechars, leaving the conversion like Linux and having regressions even on Windows.

Windows internally converts all multibyte texts to widechar anyway (NT internally uses widechars)
you can even see that in Wine and [ReactOS](https://github.com/reactos/reactos/blob/3fa57b8ff7fcee47b8e2ed869aecaf4515603f3f/win32ss/user/user32/windows/font.c#L333).

If we use multibytes, we won't have any regression from native api for linux, library api that we depend on, and
we can internally convert multibyte to widechar in Windows and cause 0 regressions (it might even be better than
the default conversion).

The only place where Widechar may cause a regression of not getting unicode text (like parsing a script) working 
in on Windows platforms that are not NT based (Win3x, Win9x),
as Win9x requires an unicode dll to work (unicows.dll, I suppose 9x doesn't really support unicode), but those systems
died long ago so there's no reason to worry about it.

Scripts: for backward compatibility with the currently existing scripts, Dhn will load, by default, scripts encoded with
Shift-JIS, it will internally convert it to UTF8 and then paste it to the parser. (NOTE: the parser might be utf-16/widechar).

If the parser is not utf-16, it would read UTF-16 texts (with BOM) and convert it to UTF-8, or leave the loading as normal.

For UTF-8 if the parser is UTF-16, it will convert UTF-8 WITH BOM texts into UTF-16, or perform no conversion.

NOTE: Dhn should be able to support both UTF-16 LE and UTF-16 BE.

The parser would mostly receive a rewrite anyway so there's free zone into that, it all depends on how much the libraries
that will compose the new parser, lexters and interpreter would like to support, I would prefer going UTF-16 rather than UTF-8 for this.
