#pragma once


#include <Evo.h>


#include "Source.h"
#include "AST.h"

namespace panther{


	class Parser{
		public:
			Parser(Source& src) : source(src) {};
			~Parser() = default;

			EVO_NODISCARD auto parse() noexcept -> bool;




		private:
			enum class ResultCode{
				Success,
				WrongType,
				Error,
			};

			template<class T>
			class Result{
				public:
					Result(ResultCode res_code) : result_code(res_code), dummy(0) {};
					Result(T&& val) : result_code(ResultCode::Success), data(std::move(val)) {};
					~Result() = default;

					EVO_NODISCARD inline auto code() const noexcept -> ResultCode { return this->result_code; };
					EVO_NODISCARD inline auto value() const noexcept -> const T& {
						evo::debugAssert(this->code == ResultCode::Success, "Attempted to get value from result that has no value");
						return this->data;
					};
			
				private:
					ResultCode result_code;

					union {
						evo::byte dummy;
						T data;
					};
			};




			EVO_NODISCARD auto parse_stmt() noexcept -> Result<AST::NodeID>;


	
		private:
			Source& source;
	};


};