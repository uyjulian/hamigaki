// vanish_routine.cpp: the routine for vanishing character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "wait_se_routine.hpp"
#include "game_system.hpp"

namespace
{

typedef hamigaki::coroutines::shared_coroutine<
    bool (game_system*, game_character*)
> coroutine_type;

class wait_se_routine_impl
{
public:
    explicit wait_se_routine_impl(const std::string& filename)
        : filename_(filename)
    {
    }

    bool operator()(
        coroutine_type::self& self, game_system* game, game_character* c) const
    {
        game->sound.play_se(filename_);

        while (game->sound.playing_se())
            boost::tie(game,c) = self.yield(true);

        return false;
    }

private:
    std::string filename_;
};

} // namespace

wait_se_routine::wait_se_routine(const std::string& filename)
    : coroutine_(wait_se_routine_impl(filename))
{
}

bool wait_se_routine::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
