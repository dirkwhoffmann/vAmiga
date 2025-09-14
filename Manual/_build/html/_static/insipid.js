(dom_loaded => {
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', dom_loaded);
    } else {
        dom_loaded();
    }
})(() => {
    'use strict';

    // make sure all scripts are re-executed when navigating to cached page
    window.onunload = () => {};

    const topbar = document.getElementById('topbar');
    const topbar_placeholder = document.getElementById('topbar-placeholder');

    const threshold = 10;

    // auto-hide topbar
    function scroll_callback(scroller) {
        let ignore_scroll = true;
        let initial;
        let scroll_timeout;
        return event => {
            window.clearTimeout(scroll_timeout);
            const current = scroller.scrollTop;
            if (current <= topbar.offsetHeight || (scroller.scrollHeight - current - scroller.clientHeight) < (scroller.clientHeight / 3)) {
                document.body.classList.remove('topbar-folded');
                ignore_scroll = true;
                return;
            } else if (ignore_scroll) {
                // We ignore single jumps
                ignore_scroll = false;
                initial = current;
            } else if (current - initial > threshold) {
                document.body.classList.add('topbar-folded');
                ignore_scroll = true;
                return;
            } else if (current - initial < -threshold) {
                document.body.classList.remove('topbar-folded');
                ignore_scroll = true;
                return;
            }
            scroll_timeout = setTimeout(() => { ignore_scroll = true; }, 66);
        };
    }

    document.addEventListener('scroll', scroll_callback(document.scrollingElement));

    const sidebar_scroller = document.querySelector('.sphinxsidebar');
    if (sidebar_scroller) {
        sidebar_scroller.addEventListener('scroll', scroll_callback(sidebar_scroller));
    }

    const div_body = document.querySelector('div.body');
    const first_section = document.querySelector('div.body .section, div.body section');
    if (first_section) {
        document.addEventListener('scroll', event => {
            if (window.pageYOffset >= div_body.offsetTop + first_section.offsetTop) {
                document.body.classList.add('scrolled');
            } else {
                document.body.classList.remove('scrolled');
            }
        });
        document.dispatchEvent(new Event('scroll'));
    }

    topbar.querySelector('.top').addEventListener('click', event => {
        window.scroll({ top: 0, behavior: 'smooth' });
        event.preventDefault();
    });

    const search_button = document.getElementById('search-button');
    if (search_button) {
        const search_form = document.getElementById('search-form');
        const search_field = search_form.querySelector('input');

        function show_search() {
            try {
                // https://readthedocs-sphinx-search.readthedocs.io/
                showSearchModal();
                return;
            } catch(e) {}
            search_form.style.display = 'flex';
            search_button.setAttribute('aria-expanded', 'true');
            search_field.focus();
            document.body.classList.remove('topbar-folded');
        }

        function hide_search() {
            search_form.style.display = 'none';
            search_button.setAttribute('aria-expanded', 'false');
            search_button.blur();
        }

        function toggle_search() {
            if (window.getComputedStyle(search_form).display === 'none') {
                show_search();
            } else {
                hide_search();
            }
        }

        search_button.addEventListener('click', toggle_search);
        if (Documentation.focusSearchBar) {
            // Monkey-patch function provided by Sphinx:
            Documentation.focusSearchBar = show_search;
        }

        search_field.addEventListener('keydown', event => {
            if (event.code === 'Escape') {
                hide_search();
                search_field.blur();
            }
        });
    }

    const fullscreen_button = document.getElementById('fullscreen-button');
    if (document.fullscreenEnabled) {
        fullscreen_button.addEventListener('click', event => {
            if (!document.fullscreenElement) {
                document.documentElement.requestFullscreen();
            } else {
                document.exitFullscreen();
            }
            fullscreen_button.blur();
            topbar_placeholder.classList.remove('fake-hover');
        });
    } else {
        fullscreen_button.remove();
    }

    topbar_placeholder.addEventListener('mouseenter', event => {
        topbar_placeholder.classList.add('fake-hover');
    });

    topbar_placeholder.addEventListener('mouseleave', event => {
        topbar_placeholder.classList.remove('fake-hover');
    });

    document.addEventListener('touchend', event => {
        if (event.touches.length > 1) { return; }
        const touch = event.touches[0];
        if (touch.clientY < topbar.offsetHeight) {
            topbar_placeholder.classList.add('fake-hover');
        } else {
            topbar_placeholder.classList.remove('fake-hover');
        }
    });
});
