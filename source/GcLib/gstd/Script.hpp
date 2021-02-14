/* ---------------------------------- */
/* 2004..2005 YT、mkm(本体担当ですよ)*/
/* このソースコードは煮るなり焼くなり好きにしろライセンスの元で配布します。*/
/* A-3、A-4に従い、このソースを組み込んだ.exeにはライセンスは適用されません。*/
/* ---------------------------------- */
/* NYSL Version 0.9982 */
/* A. 本ソフトウェアは Everyone'sWare です。このソフトを手にした一人一人が、*/
/*    ご自分の作ったものを扱うのと同じように、自由に利用することが出来ます。*/
/* A-1. フリーウェアです。作者からは使用料等を要求しません。*/
/* A-2. 有料無料や媒体の如何を問わず、自由に転載・再配布できます。*/
/* A-3. いかなる種類の 改変・他プログラムでの利用 を行っても構いません。*/
/* A-4. 変更したものや部分的に使用したものは、あなたのものになります。*/
/*      公開する場合は、あなたの名前の下で行って下さい。*/
/* B. このソフトを利用することによって生じた損害等について、作者は */
/*    責任を負わないものとします。各自の責任においてご利用下さい。*/
/* C. 著作者人格権は ○○○○ に帰属します。著作権は放棄します。*/
/* D. 以上の３項は、ソース・実行バイナリの双方に適用されます。 */
/* ---------------------------------- */

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#pragma warning(disable : 4786) //STL Warning抑止
#pragma warning(disable : 4018) //signed と unsigned の数値を比較
#pragma warning(disable : 4244) //double' から 'float' に変換
#include <list>
#include <map>
#include <string>
#include <vector>
#include <memory>

//重複宣言チェックをしない
//#define __SCRIPT_H__NO_CHECK_DUPLICATED

//-------- 汎用
namespace gstd {

template <typename T>
class lightweight_vector {
public:
	unsigned length;
	unsigned capacity;
	T* at;

	lightweight_vector()
		: length(0)
		, capacity(0)
		, at(NULL)
	{
	}

	lightweight_vector(lightweight_vector const& source);

	~lightweight_vector()
	{
		if (at != NULL) {
			delete[] at;
		}
	}

	lightweight_vector& operator=(lightweight_vector const& source);

	void expand();

	void push_back(T const& value)
	{
		if (length == capacity)
			expand();
		at[length] = value;
		++length;
	}

	void pop_back()
	{
		--length;

		T temp;
		at[length] = temp;
	}

	void clear()
	{
		length = 0;
	}

	void release()
	{
		length = 0;
		if (at != NULL) {
			delete[] at;
			at = NULL;
			capacity = 0;
		}
	}

	unsigned size() const
	{
		return length;
	}

	T& operator[](unsigned i)
	{
		return at[i];
	}

	T const& operator[](unsigned i) const
	{
		return at[i];
	}

	T& back()
	{
		return at[length - 1];
	}

	T* begin()
	{
		return &at[0];
	}

	void erase(T* pos);
	void insert(T* pos, T const& value);
};

template <typename T>
lightweight_vector<T>::lightweight_vector(lightweight_vector const& source)
{
	length = source.length;
	capacity = source.capacity;
	if (source.capacity > 0) {
		at = new T[source.capacity];
		for (int i = length - 1; i >= 0; --i)
			at[i] = source.at[i];
	} else {
		at = NULL;
	}
}

template <typename T>
lightweight_vector<T>& lightweight_vector<T>::operator=(lightweight_vector<T> const& source)
{
	if (at != NULL)
		delete[] at;
	length = source.length;
	capacity = source.capacity;
	if (source.capacity > 0) {
		at = new T[source.capacity];
		for (int i = length - 1; i >= 0; --i)
			at[i] = source.at[i];
	} else {
		at = NULL;
	}
	return *this;
}

template <typename T>
void lightweight_vector<T>::expand()
{
	if (capacity == 0) {
		// delete[] at;
		capacity = 4;
		at = new T[4];
	} else {
		capacity *= 2;
		T* n = new T[capacity];
		for (int i = length - 1; i >= 0; --i)
			n[i] = at[i];
		delete[] at;
		at = n;
	}
}

template <typename T>
void lightweight_vector<T>::erase(T* pos)
{
	--length;
	for (T* i = pos; i < at + length; ++i) {
		*i = *(i + 1);
	}
}

template <typename T>
void lightweight_vector<T>::insert(T* pos, T const& value)
{
	if (length == capacity) {
		unsigned pos_index = pos - at;
		expand();
		pos = at + pos_index;
	}
	for (T* i = at + length; i > pos; --i) {
		*i = *(i - 1);
	}
	*pos = value;
	++length;
}

//-------- ここから

class type_data {
public:
	enum type_kind {
		tk_real,
		tk_char,
		tk_boolean,
		tk_array
	};

	type_data(type_kind k, type_data* t = NULL)
		: kind(k)
		, element(t)
	{
	}

	type_data(type_data const& source)
		: kind(source.kind)
		, element(source.element)
	{
	}

	//デストラクタはデフォルトに任せる

	type_kind get_kind()
	{
		return kind;
	}

	type_data* get_element()
	{
		return element;
	}

private:
	type_kind kind;
	type_data* element;

	type_data& operator=(type_data const& source);
};

class value {
public:
	value()
		: data(NULL) {}

	value(type_data* t, long double v)
	{
		data = new body();
		data->ref_count = 1;
		data->type = t;
		data->real_value = v;
	}

	value(type_data* t, char v)
	{
		data = new body();
		data->ref_count = 1;
		data->type = t;
		data->char_value = v;
	}

	value(type_data* t, bool v)
	{
		data = new body();
		data->ref_count = 1;
		data->type = t;
		data->boolean_value = v;
	}

	value(type_data* t, std::string v);

	value(value const& source)
	{
		data = source.data;
		if (data != NULL)
			++(data->ref_count);
	}

	~value()
	{
		release();
	}

	value& operator=(value const& source)
	{
		if (source.data != NULL) {
			++(source.data->ref_count);
		}
		release();
		data = source.data;
		return *this;
	}

	bool has_data() const
	{
		return data != NULL;
	}

	void set(type_data* t, long double v)
	{
		unique();
		data->type = t;
		data->real_value = v;
	}

	void set(type_data* t, bool v)
	{
		unique();
		data->type = t;
		data->boolean_value = v;
	}

	void append(type_data* t, value const& x);
	void concatenate(value const& x);

	long double as_real() const;
	char as_char() const;
	bool as_boolean() const;
	std::string as_string() const;
	unsigned length_as_array() const;
	value const& index_as_array(unsigned i) const;
	value& index_as_array(unsigned i);
	type_data* get_type() const;

	void unique() const
	{
		if (data == NULL) {
			data = new body();
			data->ref_count = 1;
			data->type = NULL;
		} else if (data->ref_count > 1) {
			--(data->ref_count);
			data = new body(*data);
			data->ref_count = 1;
		}
	}

	void overwrite(value const& source); //危険！外から呼ぶな

private:
	inline void release()
	{
		if (data != NULL) {
			--(data->ref_count);
			if (data->ref_count == 0) {
				delete data;
			}
		}
	}
	struct body {
		int ref_count;
		type_data* type;
		lightweight_vector<value> array_value;

		union {
			long double real_value;
			char char_value;
			bool boolean_value;
		};
	};

	mutable body* data;
};

class script_engine;
class script_machine;

typedef value (*callback)(script_machine* machine, int argc, value const* argv);

struct function {
	char const* name;
	callback func;
	unsigned arguments;
};

class script_type_manager {
public:
	script_type_manager();

	type_data* get_real_type()
	{
		return real_type;
	}

	type_data* get_char_type()
	{
		return char_type;
	}

	type_data* get_boolean_type()
	{
		return boolean_type;
	}

	type_data* get_string_type()
	{
		return string_type;
	}

	type_data* get_array_type(type_data* element);

private:
	script_type_manager(script_type_manager const&);
	script_type_manager& operator=(script_type_manager const& source);

	std::list<type_data> types; //中身のポインタを使うのでアドレスが変わらないようにlist
	type_data* real_type;
	type_data* char_type;
	type_data* boolean_type;
	type_data* string_type;
};

class script_engine {
public:
	script_engine(script_type_manager* a_type_manager, std::string const& source, int funcc, function const* funcv);
	script_engine(script_type_manager* a_type_manager, std::vector<char> const& source, int funcc, function const* funcv);
	virtual ~script_engine();

	void* data; //クライアント用空間

	bool get_error()
	{
		return error;
	}

	std::string& get_error_message()
	{
		return error_message;
	}

	int get_error_line()
	{
		return error_line;
	}

	script_type_manager* get_type_manager()
	{
		return type_manager;
	}

	//compatibility
	type_data* get_real_type()
	{
		return type_manager->get_real_type();
	}

	type_data* get_char_type()
	{
		return type_manager->get_char_type();
	}

	type_data* get_boolean_type()
	{
		return type_manager->get_boolean_type();
	}

	type_data* get_array_type(type_data* element)
	{
		return type_manager->get_array_type(element);
	}

	type_data* get_string_type()
	{
		return type_manager->get_string_type();
	}

//#ifndef _MSC_VER
private:
//#endif

	//コピー、代入演算子の自動生成を無効に
	script_engine(script_engine const& source);
	script_engine& operator=(script_engine const& source);

	//エラー
	bool error;
	std::string error_message;
	int error_line;

	//型
	script_type_manager* type_manager;

	//中間コード
	enum command_kind {
		pc_assign,
		pc_assign_writable,
		pc_break_loop,
		pc_break_routine,
		pc_call,
		pc_call_and_push_result,
		pc_case_begin,
		pc_case_end,
		pc_case_if,
		pc_case_if_not,
		pc_case_next,
		pc_compare_e,
		pc_compare_g,
		pc_compare_ge,
		pc_compare_l,
		pc_compare_le,
		pc_compare_ne,
		pc_dup,
		pc_dup2,
		pc_loop_ascent,
		pc_loop_back,
		pc_loop_count,
		pc_loop_descent,
		pc_loop_if,
		pc_pop,
		pc_push_value,
		pc_push_variable,
		pc_push_variable_writable,
		pc_swap,
		pc_yield
	};

	struct block;

	struct code {
		command_kind command;
		int line; //ソースコード上の行
		value data; //pc_push_valueでpushするデータ

		union {
			struct
			{
				int level; //assign/push_variableの変数の環境位置
				unsigned variable; //assign/push_variableの変数のインデックス
			};
			struct
			{
				block* sub; //call/call_and_push_resultの飛び先
				unsigned arguments; //call/call_and_push_resultの引数の数
			};
			struct
			{
				int ip; //loop_backの戻り先
			};
		};

		code()
		{
		}

		code(int the_line, command_kind the_command)
			: line(the_line)
			, command(the_command)
		{
		}

		code(int the_line, command_kind the_command, int the_level, unsigned int the_variable)
			: line(the_line)
			, command(the_command)
			, level(the_level)
			, variable(the_variable)
		{
		}

		code(int the_line, command_kind the_command, block* the_sub, int the_arguments)
			: line(the_line)
			, command(the_command)
			, sub(the_sub)
			, arguments(the_arguments)
		{
		}

		code(int the_line, command_kind the_command, int the_ip)
			: line(the_line)
			, command(the_command)
			, ip(the_ip)
		{
		}

		code(int the_line, command_kind the_command, value const& the_data)
			: line(the_line)
			, command(the_command)
			, data(the_data)
		{
		}
	};

	enum block_kind {
		bk_normal,
		bk_loop,
		bk_sub,
		bk_function,
		bk_microthread
	};

	friend struct block;

	typedef lightweight_vector<code> codes_t;

	struct block {
		int level;
		int arguments;
		std::string name;
		callback func;
		codes_t codes;
		block_kind kind;

		block(int the_level, block_kind the_kind)
			: level(the_level)
			, arguments(0)
			, name()
			, func(NULL)
			, codes()
			, kind(the_kind)
		{
		}
	};

	std::list<block> blocks; //中身のポインタを使うのでアドレスが変わらないようにlist
	block* main_block;
	std::map<std::string, block*> events;

	block* new_block(int level, block_kind kind)
	{
		block x(level, kind);
		return &*blocks.insert(blocks.end(), x);
	}

	friend class parser;
	friend class script_machine;
};

class script_machine {
public:
	script_machine(std::shared_ptr<script_engine>& the_engine);
	virtual ~script_machine();

	void* data; //クライアント用空間

	void run();
	void call(std::string event_name);
	void resume();

	void stop()
	{
		finished = true;
		stopped = true;
	}

	bool get_stopped()
	{
		return stopped;
	}

	bool get_resuming()
	{
		return resuming;
	}

	bool get_error()
	{
		return error;
	}

	std::string& get_error_message()
	{
		return error_message;
	}

	int get_error_line()
	{
		return error_line;
	}

	void raise_error(std::string const& message)
	{
		error = true;
		error_message = message;
		finished = true;
	}
	void terminate(std::string const& message)
	{
		bTerminate = true;
		error = true;
		error_message = message;
		finished = true;
	}

	std::shared_ptr<script_engine>& get_engine()
	{
		return engine;
	}

	bool has_event(std::string event_name);

	int get_current_line();

	int get_thread_count() { return threads.size(); }

private:
	script_machine();
	script_machine(script_machine const& source);
	script_machine& operator=(script_machine const& source);

	std::shared_ptr<script_engine> engine;

	bool error;
	std::string error_message;
	int error_line;

	bool bTerminate;

	typedef lightweight_vector<value> variables_t;
	typedef lightweight_vector<value> stack_t;

	struct environment {
		environment *pred, *succ; //双方向リンクリスト
		environment* parent;
		int ref_count;
		script_engine::block* sub;
		unsigned ip;
		variables_t variables;
		stack_t stack;
		bool has_result;
	};

	std::list<environment*> call_start_parent_environment_list;
	environment* first_using_environment;
	environment* last_using_environment;
	environment* first_garbage_environment;
	environment* last_garbage_environment;
	environment* new_environment(environment* parent, script_engine::block* b);
	void dispose_environment(environment* object);

	typedef lightweight_vector<environment*> threads_t;

	threads_t threads;
	unsigned current_thread_index;
	bool finished;
	bool stopped;
	bool resuming;

	void yield()
	{
		if (current_thread_index > 0)
			--current_thread_index;
		else
			current_thread_index = threads.size() - 1;
	}

	void advance();
};

enum token_kind {
	tk_end,
	tk_invalid,
	tk_word,
	tk_real,
	tk_char,
	tk_string,
	tk_open_par,
	tk_close_par,
	tk_open_bra,
	tk_close_bra,
	tk_open_cur,
	tk_close_cur,
	tk_open_abs,
	tk_close_abs,
	tk_comma,
	tk_semicolon,
	tk_tilde,
	tk_assign,
	tk_plus,
	tk_minus,
	tk_inc,
	tk_dec,
	tk_asterisk,
	tk_slash,
	tk_percent,
	tk_caret,
	tk_e,
	tk_g,
	tk_ge,
	tk_l,
	tk_le,
	tk_ne,
	tk_exclamation,
	tk_ampersand,
	tk_and_then,
	tk_vertical,
	tk_or_else,
	tk_at,
	tk_add_assign,
	tk_subtract_assign,
	tk_multiply_assign,
	tk_divide_assign,
	tk_remainder_assign,
	tk_power_assign,
	tk_range,
	tk_ALTERNATIVE,
	tk_ASCENT,
	tk_BREAK,
	tk_CASE,
	tk_DESCENT,
	tk_ELSE,
	tk_FUNCTION,
	tk_IF,
	tk_IN,
	tk_LET,
	tk_LOCAL,
	tk_LOOP,
	tk_OTHERS,
	tk_REAL,
	tk_RETURN,
	tk_SUB,
	tk_TASK,
	tk_TIMES,
	tk_WHILE,
	tk_YIELD,
};

class scanner {
public:
	token_kind next;
	std::string word;
	long double real_value;
	char char_value;
	std::string string_value;
	int line;

	scanner(char const* source, char const* end)
		: current(source)
		, line(1)
	{
		endPoint = end;

		advance();
	}

	scanner(scanner const& source)
		: current(source.current)
		, endPoint(source.endPoint)
		, next(source.next)
		, word(source.word)
		, line(source.line)
	{
	}

	void skip();
	void advance();

	void AddLog(const char* data);

private:
	char const* current;
	char const* endPoint;

	inline char current_char();
	inline char index_from_current_char(int index);
	inline char next_char();
};

class parser {
public:
	struct symbol {
		int level;
		script_engine::block* sub;
		int variable;
	};

	struct scope : public std::map<std::string, symbol> {
		script_engine::block_kind kind;

		scope(script_engine::block_kind the_kind)
			: kind(the_kind)
		{
		}
	};

	std::vector<scope> frame;
	scanner* lex;
	script_engine* engine;
	bool error;
	std::string error_message;
	int error_line;
	std::map<std::string, script_engine::block*> events;

	parser(script_engine* e, scanner* s, int funcc, function const* funcv);

	virtual ~parser()
	{
	}

	void parse_parentheses(script_engine::block* block);
	void parse_clause(script_engine::block* block);
	void parse_prefix(script_engine::block* block);
	void parse_suffix(script_engine::block* block);
	void parse_product(script_engine::block* block);
	void parse_sum(script_engine::block* block);
	void parse_comparison(script_engine::block* block);
	void parse_logic(script_engine::block* block);
	void parse_expression(script_engine::block* block);
	int parse_arguments(script_engine::block* block);
	void parse_statements(script_engine::block* block);
	void parse_inline_block(script_engine::block* block, script_engine::block_kind kind);
	void parse_block(script_engine::block* block, std::vector<std::string> const* args, bool adding_result);

private:
	void register_function(function const& func);
	symbol* search(std::string const& name);
	symbol* search_result();
	void scan_current_scope(int level, std::vector<std::string> const* args, bool adding_result);
	void write_operation(script_engine::block* block, char const* name, int clauses);

	typedef script_engine::code code;
};


template <int num>
class constant {
public:
	static value func(script_machine* machine, int argc, value const* argv)
	{
		return value(machine->get_engine()->get_real_type(), (long double)num);
	}
};

} // namespace gstd

#endif
