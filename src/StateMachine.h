#include <variant>
#include  <utility>
#include <string>

// T is a typelist
template <typename... States>
class StateMachine
{
public:
	template<typename State, typename... Args>
	void ChangeState(Args&&... args)
	{
		m_state = State(std::forward<Args>(args)...);
	}

	template <class Visitor>
	void VisitState(Visitor& visitor) const
	{
		std::visit(
			[&](auto& state) {visitor.Visit(state); },
			m_state
		);
	}

	template <typename... Args>
	void Update(Args&&... args) {
		auto passEventToState = [&](auto& state) {
			state.Update(std::forward<Args>(args)...);
		};
		std::visit(passEventToState, m_state);
	}

	template <typename... Args>
	void ReceiveMessage(Args&&... args) {
		auto passEventToState = [&](auto& state) {
			state.ReceiveMessage(std::forward<Args>(args)...);
		};
		std::visit(passEventToState, m_state);
	}

private:
	std::variant<States...> m_state;
};