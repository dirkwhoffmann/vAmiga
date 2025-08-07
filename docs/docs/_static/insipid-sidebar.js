(dom_loaded => {
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', dom_loaded);
    } else {
        dom_loaded();
    }
})(() => {
    'use strict';

    const sidebar = document.querySelector('.sphinxsidebar');
    const sidebar_tabbable = sidebar.querySelectorAll('input, textarea, select, button, a[href], area[href], iframe');
    const sidebar_button = document.getElementById('sidebar-button');
    const sidebar_checkbox = document.getElementById('sidebar-checkbox');
    const topbar = document.getElementById('topbar');
    const overlay = document.getElementById('overlay');
    const root = document.documentElement;

    sidebar.setAttribute('id', 'sphinxsidebar');  // for aria-controls

    Element.prototype.css = function (name, ...value) {
        if (value.length) {
            this.style.setProperty(name, ...value);
        } else {
            return window.getComputedStyle(this).getPropertyValue(name);
        }
    }

    function updateSidebarAttributesVisible() {
        sidebar_button.setAttribute('title', "Collapse sidebar");
        sidebar_button.setAttribute('aria-label', "Collapse sidebar");
        sidebar_button.setAttribute('aria-expanded', true);
        sidebar.setAttribute('aria-hidden', false);
        sidebar_tabbable.forEach(el => el.setAttribute('tabindex', 0));
    }

    function updateSidebarAttributesHidden() {
        sidebar_button.setAttribute('title', "Expand sidebar");
        sidebar_button.setAttribute('aria-label', "Expand sidebar");
        sidebar_button.setAttribute('aria-expanded', false);
        sidebar.setAttribute('aria-hidden', true);
        sidebar_tabbable.forEach(el => el.setAttribute('tabindex', -1));
    }

    sidebar.setAttribute('tabindex', -1);

    function store(key, value) {
        try {
            localStorage.setItem(key, value);
        } catch (e) {
        }
    }

    sidebar_checkbox.addEventListener('change', event => {
        if (event.target.checked) {
            updateSidebarAttributesVisible();
            store('sphinx-sidebar', 'visible');
            document.body.classList.remove('topbar-folded');
            sidebar.focus({preventScroll: true});
            sidebar.blur();
        } else {
            updateSidebarAttributesHidden();
            store('sphinx-sidebar', 'hidden');
            if (document.scrollingElement.scrollTop < topbar.offsetHeight) {
                document.body.classList.remove('topbar-folded');
            } else {
                document.body.classList.add('topbar-folded');
            }
            document.scrollingElement.focus({preventScroll: true});
            document.scrollingElement.blur();
        }
    });

    if (sidebar_checkbox.checked) {
        updateSidebarAttributesVisible();
    } else {
        updateSidebarAttributesHidden();
    }

    function show() {
        sidebar_checkbox.checked = true;
        sidebar_checkbox.dispatchEvent(new Event('change'));
    }

    function hide() {
        sidebar_checkbox.checked = false;
        sidebar_checkbox.dispatchEvent(new Event('change'));
    }

    sidebar_button.addEventListener('keydown', event => {
        if (event.code === 'Enter' || event.code === 'Space') {
            sidebar_button.click();
            event.preventDefault();
        }
    });

    let touchstart;

    document.addEventListener('touchstart', event => {
        if (event.touches.length > 1) { return; }
        const touch = event.touches[0];
        if (sidebar_checkbox.checked) {
            if (touch.clientX > sidebar.offsetWidth) {
                return;
            }
        } else {
            if (touch.clientX > 20) {
                return;
            }
        }
        touchstart = {
            x: touch.clientX,
            y: touch.clientY,
            t: Date.now(),
        };
    });

    document.addEventListener('touchend', event => {
        if (!touchstart) { return; }
        if (event.touches.length > 0 || event.changedTouches.length > 1) {
            touchstart = null;
            return;
        }
        const touch = event.changedTouches[0];
        const x = touch.clientX;
        const y = touch.clientY;
        const x_diff = x - touchstart.x;
        const y_diff = y - touchstart.y;
        const t_diff = Date.now() - touchstart.t;
        if (t_diff < 400 && Math.abs(x_diff) > Math.max(100, Math.abs(y_diff))) {
            if (x_diff > 0) {
                if (!sidebar_checkbox.checked) {
                    show();
                }
            } else {
                if (sidebar_checkbox.checked) {
                    hide();
                }
            }
        }
        touchstart = null;
    });

    const sidebar_resize_handles = document.querySelectorAll('.sidebar-resize-handle');
    sidebar_resize_handles.forEach(el => {
        el.addEventListener('mousedown', event => {
            window.addEventListener('mousemove', resize_mouse);
            window.addEventListener('mouseup', stop_resize_mouse);
            document.body.classList.add('sidebar-resizing');
            event.preventDefault();  // Prevent unwanted text selection while resizing
        });
        el.addEventListener('touchstart', event => {
            if (event.touches.length > 1) { return; }
            window.addEventListener('touchmove', resize_touch);
            window.addEventListener('touchend', stop_resize_touch);
            document.body.classList.add('sidebar-resizing');
            event.preventDefault();  // Prevent unwanted text selection while resizing
        });
    });

    let ignore_resize = false;

    function resize_base(event) {
        if (ignore_resize) { return; }
        const window_width = window.innerWidth;
        const width = event.clientX;
        if (width > window_width) {
            root.css('--sidebar-width', window_width + 'px');
        } else if (width > 10) {
            root.css('--sidebar-width', width + 'px');
        } else {
            ignore_resize = true;
            hide();
        }
    }

    function resize_mouse(event) {
        resize_base(event);
    }

    function resize_touch(event) {
        if (event.touches.length > 1) { return; }
        resize_base(event.touches[0]);
    }

    function stop_resize_base() {
        if (ignore_resize) {
            root.css('--sidebar-width', '19rem');
            ignore_resize = false;
        }
        store('sphinx-sidebar-width', root.css('--sidebar-width'));
        document.body.classList.remove('sidebar-resizing');
    }

    function stop_resize_mouse(event) {
        window.removeEventListener('mousemove', resize_mouse);
        window.removeEventListener('mouseup', stop_resize_mouse);
        stop_resize_base();
    }

    function stop_resize_touch(event) {
        if (event.touches.length > 0 || event.changedTouches.length > 1) {
            return;
        }
        window.removeEventListener('touchmove', resize_touch);
        window.removeEventListener('touchend', stop_resize_touch);
        stop_resize_base();
    }

    window.addEventListener('resize', event => {
        const window_width = window.innerWidth;
        if (window_width < sidebar.offsetWidth) {
            root.css('--sidebar-width', window_width + 'px');
        }
    });

    // This is part of the sidebar code because it only affects the sidebar
    if (window.ResizeObserver) {
        const resizeObserver = new ResizeObserver(entries => {
            for (let entry of entries) {
                let height;
                if (entry.borderBoxSize && entry.borderBoxSize.length > 0) {
                    height = entry.borderBoxSize[0].blockSize;
                } else {
                    height = entry.contentRect.height;
                }
                root.css('--topbar-height', height + 'px');
            }
        });
        resizeObserver.observe(topbar);
    }

    let current = [];
    let links = [];

    document.querySelectorAll('.sphinxsidebar *').forEach(el => {
        let link = el.querySelector(':scope > a[href^="#"]');
        if (link) {
            el.classList.add('current-page');
            current.push(el);
            links.push(link);
        }
    });
    const small_screen = window.matchMedia('(max-width: 39rem)');

    if (current.length === 1 && current[0].childElementCount === 1 && small_screen.matches) {
        hide();
    }
    const bottom_space = 0;

    if (current.length) {
        const top = current[0].getBoundingClientRect().top;
        const bottom = current[current.length - 1].getBoundingClientRect().bottom;
        if (top < topbar.offsetHeight || bottom > (sidebar.offsetHeight - bottom_space)) {
            current[0].scrollIntoView(true);
        }
    }

    let sections = new Map();

    const intersection_callback = (entries, observer) => {
        entries.forEach(entry => {
            let link = sections.get(entry.target);
            if (entry.isIntersecting) {
                link.classList.add('in-view');
            } else {
                link.classList.remove('in-view');
            }
        });
    };

    const intersection_observer = new IntersectionObserver(intersection_callback, {
        root: null,
        // NB: This uses the initial topbar height, later changes are ignored:
        rootMargin: -topbar.offsetHeight + 'px 0px 0px 0px',
        threshold: 0.0,
    });

    links.forEach(link => {
        let section;
        let id = link.hash;
        if (id) {
            id = id.slice(1);
            section = document.getElementById(decodeURI(id));
            // Detect API doc headers:
            let single_definition_term = (
                section.nodeName == 'DT' &&
                section.nextElementSibling.nodeName == 'DD' &&
                !section.nextElementSibling.nextElementSibling &&
                section.parentElement.nodeName == 'DL');
            if (single_definition_term) {
                // The <dl> contains only a single <dt> + <dd>,
                // therefore we can observe the whole <dl>.
                section = section.parentElement;
            }
        } else {
            // NB: The first section has no hash, so we don't know its ID:
            section = document.querySelector('div.body .section, div.body section');
        }
        sections.set(section, link);
        intersection_observer.observe(section);
        link.addEventListener('click', event => {
            if (small_screen.matches) {
                hide();
            }
        });
    });
});

