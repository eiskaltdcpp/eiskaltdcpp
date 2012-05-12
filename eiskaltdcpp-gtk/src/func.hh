/*
 * Copyright © 2004-2010 Jens Oknelid, paskharen@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#pragma once

class FuncBase
{
	public:
		FuncBase() {}
		virtual ~FuncBase() {}
		virtual void call() = 0;
		virtual const std::string& getID() = 0;
};

template<class c>
class Func0: public FuncBase
{
	public:
		Func0(c *obj, void (c::*func)())
		{
			this->obj = obj;
			this->func = func;
		}

		void call() {
			(*obj.*func)();
		}

		const std::string& getID()
		{
			return obj->getID();
		}

	private:
		c *obj;
		void (c::*func)();
};

template<class c, typename p1>
class Func1: public FuncBase
{
	public:
		Func1(c *obj, void (c::*func)(p1), p1 param1):
			_param1(param1)
		{
			this->obj = obj;
			this->func = func;
		}

		void call() {
			(*obj.*func)(_param1);
		}

		const std::string& getID()
		{
			return obj->getID();
		}

	private:
		c *obj;
		void (c::*func)(p1);
		p1 _param1;
};

template<class c, typename p1, typename p2>
class Func2: public FuncBase
{
	public:
		Func2(c *obj, void (c::*func)(p1, p2), p1 param1, p2 param2):
			_param1(param1),
			_param2(param2)
		{
			this->obj = obj;
			this->func = func;
		}

		void call() {
			(*obj.*func)(_param1, _param2);
		}

		const std::string& getID()
		{
			return obj->getID();
		}

	private:
		c *obj;
		void (c::*func)(p1, p2);
		p1 _param1;
		p2 _param2;
};

template<class c, typename p1, typename p2, typename p3>
class Func3: public FuncBase
{
	public:
		Func3(c *obj, void (c::*func)(p1, p2, p3), p1 param1, p2 param2, p3 param3):
			_param1(param1),
			_param2(param2),
			_param3(param3)
		{
			this->obj = obj;
			this->func = func;
		}

		void call() {
			(*obj.*func)(_param1, _param2, _param3);
		}

		const std::string& getID()
		{
			return obj->getID();
		}

	private:
		c *obj;
		void (c::*func)(p1, p2, p3);
		p1 _param1;
		p2 _param2;
		p3 _param3;
};

template<class c, typename p1, typename p2, typename p3, typename p4>
class Func4: public FuncBase
{
	public:
		Func4(c *obj, void (c::*func)(p1, p2, p3, p4),
			p1 param1, p2 param2, p3 param3, p4 param4):
			_param1(param1),
			_param2(param2),
			_param3(param3),
			_param4(param4)
		{
			this->obj = obj;
			this->func = func;
		}

		void call() {
			(*obj.*func)(_param1, _param2, _param3, _param4);
		}

		const std::string& getID()
		{
			return obj->getID();
		}

	private:
		c *obj;
		void (c::*func)(p1, p2, p3, p4);
		p1 _param1;
		p2 _param2;
		p3 _param3;
		p4 _param4;
};

template<class c, typename p1, typename p2, typename p3, typename p4, typename p5>
class Func5: public FuncBase
{
	public:
		Func5(c *obj, void (c::*func)(p1, p2, p3, p4, p5),
			p1 param1, p2 param2, p3 param3, p4 param4, p5 param5):
			_param1(param1),
			_param2(param2),
			_param3(param3),
			_param4(param4),
			_param5(param5)
		{
			this->obj = obj;
			this->func = func;
		}

		void call() {
			(*obj.*func)(_param1, _param2, _param3, _param4, _param5);
		}

		const std::string& getID()
		{
			return obj->getID();
		}

	private:
		c *obj;
		void (c::*func)(p1, p2, p3, p4, p5);
		p1 _param1;
		p2 _param2;
		p3 _param3;
		p4 _param4;
		p5 _param5;
};

template<class c, typename p1, typename p2, typename p3, typename p4, typename p5, typename p6>
class Func6: public FuncBase
{
	public:
		Func6(c *obj, void (c::*func)(p1, p2, p3, p4, p5, p6),
			p1 param1, p2 param2, p3 param3, p4 param4, p5 param5, p6 param6):
			_param1(param1),
			_param2(param2),
			_param3(param3),
			_param4(param4),
			_param5(param5),
			_param6(param6)
		{
			this->obj = obj;
			this->func = func;
		}

		void call() {
			(*obj.*func)(_param1, _param2, _param3, _param4, _param5, _param6);
		}

		const std::string& getID()
		{
			return obj->getID();
		}

	private:
		c *obj;
		void (c::*func)(p1, p2, p3, p4, p5, p6);
		p1 _param1;
		p2 _param2;
		p3 _param3;
		p4 _param4;
		p5 _param5;
		p6 _param6;
};

template<class c, typename p1, typename p2, typename p3, typename p4, typename p5, typename p6, typename p7>
class Func7: public FuncBase
{
	public:
		Func7(c *obj, void (c::*func)(p1, p2, p3, p4, p5, p6, p7),
			p1 param1, p2 param2, p3 param3, p4 param4, p5 param5, p6 param6, p7 param7):
			_param1(param1),
			_param2(param2),
			_param3(param3),
			_param4(param4),
			_param5(param5),
			_param6(param6),
			_param7(param7)
		{
			this->obj = obj;
			this->func = func;
		}

		void call() {
			(*obj.*func)(_param1, _param2, _param3, _param4, _param5, _param6, _param7);
		}

		const std::string& getID()
		{
			return obj->getID();
		}

	private:
		c *obj;
		void (c::*func)(p1, p2, p3, p4, p5, p6, p7);
		p1 _param1;
		p2 _param2;
		p3 _param3;
		p4 _param4;
		p5 _param5;
		p6 _param6;
		p7 _param7;
};

template<class c, typename p1, typename p2, typename p3, typename p4,
	typename p5, typename p6, typename p7, typename p8>
class Func8: public FuncBase
{
	public:
		Func8(c *obj, void (c::*func)(p1, p2, p3, p4, p5, p6, p7, p8),
			p1 param1, p2 param2, p3 param3, p4 param4, p5 param5, p6 param6,
			p7 param7, p8 param8):
			_param1(param1),
			_param2(param2),
			_param3(param3),
			_param4(param4),
			_param5(param5),
			_param6(param6),
			_param7(param7),
			_param8(param8)
		{
			this->obj = obj;
			this->func = func;
		}

		void call() {
			(*obj.*func)(_param1, _param2, _param3, _param4, _param5, _param6,
				_param7, _param8);
		}

		const std::string& getID()
		{
			return obj->getID();
		}

	private:
		c *obj;
		void (c::*func)(p1, p2, p3, p4, p5, p6, p7, p8);
		p1 _param1;
		p2 _param2;
		p3 _param3;
		p4 _param4;
		p5 _param5;
		p6 _param6;
		p7 _param7;
		p8 _param8;
};

template<class c, typename p1, typename p2, typename p3, typename p4,
	typename p5, typename p6, typename p7, typename p8, typename p9>
class Func9: public FuncBase
{
	public:
		Func9(c *obj, void (c::*func)(p1, p2, p3, p4, p5, p6, p7, p8, p9),
			p1 param1, p2 param2, p3 param3, p4 param4, p5 param5, p6 param6,
			p7 param7, p8 param8, p9 param9):
			_param1(param1),
			_param2(param2),
			_param3(param3),
			_param4(param4),
			_param5(param5),
			_param6(param6),
			_param7(param7),
			_param8(param8),
			_param9(param9)
		{
			this->obj = obj;
			this->func = func;
		}

		void call() {
			(*obj.*func)(_param1, _param2, _param3, _param4, _param5, _param6,
				_param7, _param8, _param9);
		}

		const std::string& getID()
		{
			return obj->getID();
		}

	private:
		c *obj;
		void (c::*func)(p1, p2, p3, p4, p5, p6, p7, p8, p9);
		p1 _param1;
		p2 _param2;
		p3 _param3;
		p4 _param4;
		p5 _param5;
		p6 _param6;
		p7 _param7;
		p8 _param8;
		p9 _param9;
};
