/*
 * SPDX-FileCopyrightText: 2023-2023 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "waylandimserverbase.h"
#include <optional>
#include <string>
#include <wayland-client-core.h>
#include "fcitx-utils/event.h"
#include "fcitx-utils/utf8.h"
#include "waylandim.h"

namespace fcitx {

WaylandIMServerBase::WaylandIMServerBase(wl_display *display, FocusGroup *group,
                                         const std::string &name,
                                         WaylandIMModule *waylandim)
    : group_(group), name_(name), parent_(waylandim),
      display_(
          static_cast<wayland::Display *>(wl_display_get_user_data(display))) {}

void WaylandIMServerBase::deferredFlush() {
    if (deferEvent_) {
        return;
    }

    deferEvent_ =
        parent_->instance()->eventLoop().addDeferEvent([this](EventSource *) {
            display_->flush();
            deferEvent_.reset();
            return true;
        });
}

std::optional<std::string>
WaylandIMServerBase::mayCommitAsText(const Key &key, uint32_t state) const {
    KeyStates nonShiftMask =
        KeyStates(KeyState::SimpleMask) & (~KeyStates(KeyState::Shift));
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED &&
        !*parent_->config().preferKeyEvent) {
        auto utf32 = Key::keySymToUnicode(key.sym());
        bool chToIgnore = (utf32 == '\n' || utf32 == '\b' || utf32 == '\r' ||
                           utf32 == '\t' || utf32 == '\033' || utf32 == '\x7f');
        if (!key.states().testAny(nonShiftMask) && utf32 && !chToIgnore) {
            return utf8::UCS4ToUTF8(utf32);
        }
    }
    return std::nullopt;
}

} // namespace fcitx
