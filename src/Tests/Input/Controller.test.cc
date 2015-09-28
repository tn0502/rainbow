// Copyright (c) 2010-15 Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#include <gtest/gtest.h>

#include "Input/Controller.h"
#include "Input/Input.h"
#include "Input/InputListener.h"

namespace
{
	class ControllerTest : public InputListener, public ::testing::Test
	{
	public:
		struct Events
		{
			static const unsigned int AxisMotion    = 1u << 0;
			static const unsigned int ButtonDown    = 1u << 1;
			static const unsigned int ButtonUp      = 1u << 2;
			static const unsigned int Connected     = 1u << 3;
			static const unsigned int Disconnected  = 1u << 4;
		};

		ControllerTest() : flags_(0) {}

		ControllerAxisMotion axis_motion() const
		{
			return ControllerAxisMotion(id(), Controller::Axis::LeftX, 255, 1);
		}

		ControllerButton button() const
		{
			return ControllerButton(id(), Controller::Button::A, 1);
		}

		uint32_t id() const { return 1; }

		bool is_triggered(const unsigned int flags) { return flags_ & flags; }

	protected:
		Input input;

		void SetUp() override { input.subscribe(this); }

	private:
		unsigned int flags_;

		bool on_controller_axis_motion_impl(
		    const ControllerAxisMotion& motion) override
		{
			[this, &motion]
			{
				const auto& expected = axis_motion();
				ASSERT_EQ(expected.id, motion.id);
				ASSERT_EQ(expected.axis, motion.axis);
				ASSERT_EQ(expected.value, motion.value);
				ASSERT_EQ(expected.timestamp, motion.timestamp);
			}();

			flags_ |= Events::AxisMotion;
			return true;
		}

		bool
		on_controller_button_down_impl(const ControllerButton& btn) override
		{
			[this, &btn]
			{
				const auto& expected = button();
				ASSERT_EQ(expected.id, btn.id);
				ASSERT_EQ(expected.button, btn.button);
				ASSERT_EQ(expected.timestamp, btn.timestamp);
			}();

			flags_ |= Events::ButtonDown;
			return true;
		}

		bool on_controller_button_up_impl(const ControllerButton& btn) override
		{
			[this, &btn]
			{
				const auto& expected = button();
				ASSERT_EQ(expected.id, btn.id);
				ASSERT_EQ(expected.button, btn.button);
				ASSERT_EQ(expected.timestamp, btn.timestamp);
			}();

			flags_ |= Events::ButtonUp;
			return true;
		}

		bool on_controller_connected_impl(const unsigned int i) override
		{
			[this, i] { ASSERT_EQ(id(), i); }();

			flags_ |= Events::Connected;
			return true;
		}

		bool on_controller_disconnected_impl(const unsigned int i) override
		{
			[this, i] { ASSERT_EQ(id(), i); }();

			flags_ |= Events::Disconnected;
			return true;
		}
	};
}

TEST_F(ControllerTest, IsInvalidByDefault)
{
	const ControllerAxisMotion axis;

	ASSERT_EQ(0u, axis.id);
	ASSERT_EQ(Controller::Axis::Invalid, axis.axis);
	ASSERT_EQ(0, axis.value);
	ASSERT_EQ(0ull, axis.timestamp);

	const ControllerButton button;

	ASSERT_EQ(0u, button.id);
	ASSERT_EQ(Controller::Button::Invalid, button.button);
	ASSERT_EQ(0ull, button.timestamp);
}

TEST_F(ControllerTest, on_controller_axis_motion)
{
	input.on_controller_axis_motion(axis_motion());
	ASSERT_TRUE(is_triggered(Events::AxisMotion));
	ASSERT_FALSE(is_triggered(0xff ^ Events::AxisMotion));
}

TEST_F(ControllerTest, on_controller_button_down)
{
	input.on_controller_button_down(button());
	ASSERT_TRUE(is_triggered(Events::ButtonDown));
	ASSERT_FALSE(is_triggered(0xff ^ Events::ButtonDown));
}

TEST_F(ControllerTest, on_controller_button_up)
{
	input.on_controller_button_up(button());
	ASSERT_TRUE(is_triggered(Events::ButtonUp));
	ASSERT_FALSE(is_triggered(0xff ^ Events::ButtonUp));
}

TEST_F(ControllerTest, on_controller_connected)
{
	input.on_controller_connected(id());
	ASSERT_TRUE(is_triggered(Events::Connected));
	ASSERT_FALSE(is_triggered(0xff ^ Events::Connected));
}

TEST_F(ControllerTest, on_controller_disconnected)
{
	input.on_controller_disconnected(id());
	ASSERT_TRUE(is_triggered(Events::Disconnected));
	ASSERT_FALSE(is_triggered(0xff ^ Events::Disconnected));
}
