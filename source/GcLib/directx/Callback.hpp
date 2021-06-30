#pragma once

class BgfxCallback : public bgfx::CallbackI
{
public:
	virtual ~BgfxCallback() {}

	/// This callback is called on unrecoverable errors.
	/// It's not safe to continue (Exluding _code `Fatal::DebugCheck`),
	/// inform the user and terminate the application.
	///
	/// @param[in] _filePath File path where fatal message was generated.
	/// @param[in] _line Line where fatal message was generated.
	/// @param[in] _code Fatal error code.
	/// @param[in] _str More information about error.
	///
	/// @remarks
	///   Not thread safe and it can be called from any thread.
	///
	/// @attention C99 equivalent is `bgfx_callback_vtbl.fatal`.
	///
	void fatal(
				const char* _filePath
			, uint16_t _line
			, bgfx::Fatal::Enum _code
			, const char* _str
			)
	{
		std::string szMsg = "";

#ifdef _DEBUG
		szMsg = gstd::StringUtility::Format("[%s:%u] >>", _filePath, _line);
#endif

		const char* szBgfxMsg;
		switch (_code)
		{
		case bgfx::Fatal::DebugCheck:
			szBgfxMsg = "Bug check";
			break;
		case bgfx::Fatal::DeviceLost:
			szBgfxMsg = "Device lost";
			break;
		case bgfx::Fatal::InvalidShader:
			szBgfxMsg = "Invalid shader";
			break;
		case bgfx::Fatal::UnableToCreateTexture:
			szBgfxMsg = "Texture creation failed";
			break;
		case bgfx::Fatal::UnableToInitialize:
			szBgfxMsg = "Initialization failed";
			break;
		default:
			szBgfxMsg = "Unknown error";
			break;
		}

		szMsg += gstd::StringUtility::Format("%s\n%s", szBgfxMsg, _str);

		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Engine error", szMsg.c_str(), nullptr);
	}

	/// Print debug message.
	///
	/// @param[in] _filePath File path where debug message was generated.
	/// @param[in] _line Line where debug message was generated.
	/// @param[in] _format `printf` style format.
	/// @param[in] _argList Variable arguments list initialized with
	///   `va_start`.
	///
	/// @remarks
	///   Not thread safe and it can be called from any thread.
	///
	/// @attention C99 equivalent is `bgfx_callback_vtbl.trace_vargs`.
	///
	void traceVargs(
			const char* _filePath
		, uint16_t _line
		, const char* _format
		, va_list _argList
		)
	{
#ifdef _DEBUG
		char buffer[1024];
		std::string szStr = gstd::StringUtility::Format("BGFX TRACE [%s:%u] >> ", _filePath, _line);
		_vsnprintf(buffer, 1024, _format, _argList);
		szStr += buffer;

#ifdef _WIN32
		OutputDebugStringA(szStr.c_str());
#else
		fprintf(stdout, szStr.c_str());
#endif

#endif
	}

	/// Profiler region begin.
	///
	/// @param[in] _name Region name, contains dynamic string.
	/// @param[in] _abgr Color of profiler region.
	/// @param[in] _filePath File path where `profilerBegin` was called.
	/// @param[in] _line Line where `profilerBegin` was called.
	///
	/// @remarks
	///   Not thread safe and it can be called from any thread.
	///
	/// @attention C99 equivalent is `bgfx_callback_vtbl.profiler_begin`.
	///
	void profilerBegin(
			const char* _name
		, uint32_t _abgr
		, const char* _filePath
		, uint16_t _line
	) {}

	/// Profiler region begin with string literal name.
	///
	/// @param[in] _name Region name, contains string literal.
	/// @param[in] _abgr Color of profiler region.
	/// @param[in] _filePath File path where `profilerBeginLiteral` was called.
	/// @param[in] _line Line where `profilerBeginLiteral` was called.
	///
	/// @remarks
	///   Not thread safe and it can be called from any thread.
	///
	/// @attention C99 equivalent is `bgfx_callback_vtbl.profiler_begin_literal`.
	///
	void profilerBeginLiteral(
			const char* _name
		, uint32_t _abgr
		, const char* _filePath
		, uint16_t _line
	) {}

	/// Profiler region end.
	///
	/// @remarks
	///   Not thread safe and it can be called from any thread.
	///
	/// @attention C99 equivalent is `bgfx_callback_vtbl.profiler_end`.
	///
	void profilerEnd() {}

	/// Returns the size of a cached item. Returns 0 if no cached item was
	/// found.
	///
	/// @param[in] _id Cache id.
	/// @returns Number of bytes to read.
	///
	/// @attention C99 equivalent is `bgfx_callback_vtbl.cache_read_size`.
	///
	uint32_t cacheReadSize(uint64_t _id) { return 0; }

	/// Read cached item.
	///
	/// @param[in] _id Cache id.
	/// @param[in] _data Buffer where to read data.
	/// @param[in] _size Size of data to read.
	///
	/// @returns True if data is read.
	///
	/// @attention C99 equivalent is `bgfx_callback_vtbl.cache_read`.
	///
	bool cacheRead(uint64_t _id, void* _data, uint32_t _size) { return false; }

	/// Write cached item.
	///
	/// @param[in] _id Cache id.
	/// @param[in] _data Data to write.
	/// @param[in] _size Size of data to write.
	///
	/// @attention C99 equivalent is `bgfx_callback_vtbl.cache_write`.
	///
	void cacheWrite(uint64_t _id, const void* _data, uint32_t _size) {}

	/// Screenshot captured. Screenshot format is always 4-byte BGRA.
	///
	/// @param[in] _filePath File path.
	/// @param[in] _width Image width.
	/// @param[in] _height Image height.
	/// @param[in] _pitch Number of bytes to skip between the start of
	///   each horizontal line of the image.
	/// @param[in] _data Image data.
	/// @param[in] _size Image size.
	/// @param[in] _yflip If true, image origin is bottom left.
	///
	/// @attention C99 equivalent is `bgfx_callback_vtbl.screen_shot`.
	///
	void screenShot(
			const char* _filePath
		, uint32_t _width
		, uint32_t _height
		, uint32_t _pitch
		, const void* _data
		, uint32_t _size
		, bool _yflip
	) {}

	/// Called when a video capture begins.
	///
	/// @param[in] _width Image width.
	/// @param[in] _height Image height.
	/// @param[in] _pitch Number of bytes to skip between the start of
	///   each horizontal line of the image.
	/// @param[in] _format Texture format. See: `TextureFormat::Enum`.
	/// @param[in] _yflip If true, image origin is bottom left.
	///
	/// @attention C99 equivalent is `bgfx_callback_vtbl.capture_begin`.
	///
	void captureBegin(
			uint32_t _width
		, uint32_t _height
		, uint32_t _pitch
		, bgfx::TextureFormat::Enum _format
		, bool _yflip
	) {}

	/// Called when a video capture ends.
	///
	/// @attention C99 equivalent is `bgfx_callback_vtbl.capture_end`.
	///
	void captureEnd() {}

	/// Captured frame.
	///
	/// @param[in] _data Image data.
	/// @param[in] _size Image size.
	///
	/// @attention C99 equivalent is `bgfx_callback_vtbl.capture_frame`.
	///
	void captureFrame(const void* _data, uint32_t _size) {}
};
