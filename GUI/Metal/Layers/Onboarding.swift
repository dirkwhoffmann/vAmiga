
// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
class OnboardingLayerView: NSView {

    override func mouseDown(with event: NSEvent) {

        print("OnboardingLayerView: mouseDown")
    }

    override func mouseUp(with event: NSEvent) {

        print("OnboardingLayerView: mouseUp")
    }

    private var bgLayer: CALayer?

    override func viewDidMoveToWindow() {

        /*
        super.viewDidMoveToWindow()
        wantsLayer = true

        if let layer = self.layer, bgLayer == nil {

            let background = CALayer()
            background.contents = NSImage(named: "a1000board")
            background.contentsGravity = .resizeAspectFill
            background.autoresizingMask = [.layerWidthSizable, .layerHeightSizable]
            layer.insertSublayer(background, at: 0)
            bgLayer = background
        }
         */
    }

    override func layout() {

        super.layout()
        bgLayer?.frame = CGRect(x: 0, y: 0,
                                width: bounds.width,
                                height: bounds.height)
    }
}

@MainActor
class OnboardingPageContainerView: NSView {

    /*
    private var bgLayer: CALayer?

    override func viewDidMoveToWindow() {

        super.viewDidMoveToWindow()
        wantsLayer = true

        if let layer = self.layer, bgLayer == nil {

            let background = CALayer()
            background.contents = NSImage(named: "vAmigaBg")
            background.contentsGravity = .resizeAspectFill
            background.autoresizingMask = [.layerWidthSizable, .layerHeightSizable]
            layer.insertSublayer(background, at: 0)
            bgLayer = background
        }
    }

    override func layout() {

        super.layout()
        bgLayer?.frame = CGRect(x: 0, y: 0,
                                width: bounds.width,
                                height: bounds.height - 32)
    }
    */
}

class OnboardingLayerViewController: NSViewController {

    @IBOutlet weak var pageContainerView: OnboardingPageContainerView!
    @IBOutlet weak var pageDotIndicator: PageDotsIndicator!

    // var pageController: NSPageController!
    private var pages: [NSViewController] = []
    private var currentPageIndex: Int = 0 {
        didSet { pageDotIndicator.currentPage = currentPageIndex }
    }

    override func viewDidLoad() {

        super.viewDidLoad()
    }

    override func viewDidAppear() {

        func instantiate(_ id: String) -> NSViewController {

            let storyboard = NSStoryboard(name: "Onboarding", bundle: nil)
            return storyboard.instantiateController(withIdentifier: id) as! NSViewController
        }

        super.viewDidAppear()
        pages = [instantiate("Step1"), instantiate("Step2")]
        pageDotIndicator.numberOfPages = pages.count

        showPage(at: 0)
    }

    private func showPage(at index: Int, animated: Bool = true) {

        guard pages.indices.contains(index) else { return }

        let newPage = pages[index]
        let oldPage = children.isEmpty ? nil : children[0]

        addChild(newPage)
        newPage.view.frame = pageContainerView.bounds
        newPage.view.autoresizingMask = [.width, .height]

        if let oldPage = oldPage {

            // Transition from the old to the new page
            if animated {

                let options: NSViewController.TransitionOptions =
                [index > currentPageIndex ? .slideLeft : .slideRight, .allowUserInteraction]
                NSAnimationContext.runAnimationGroup({ context in

                    context.duration = 0.8
                    context.timingFunction = CAMediaTimingFunction(name: .easeInEaseOut)
                    self.transition(from: oldPage,
                                    to: newPage,
                                    options: options,
                                    completionHandler: nil)

                }, completionHandler: {

                    Task { @MainActor in oldPage.removeFromParent() }
                })

            } else {

                transition(from: oldPage, to: newPage, options: [], completionHandler: nil)
                oldPage.removeFromParent()
            }

        } else {

            // First page, just add it
            pageContainerView.addSubview(newPage.view)
        }

        currentPageIndex = index
    }

    @IBAction func nextPage(_ sender: Any?) {

        let nextIndex = currentPageIndex + 1
        if pages.indices.contains(nextIndex) {
            showPage(at: nextIndex)
        }
    }

    @IBAction func previousPage(_ sender: Any?) {

        let prevIndex = currentPageIndex - 1
        if pages.indices.contains(prevIndex) {
            showPage(at: prevIndex)
        }
    }
}

@MainActor
class Onboarding: Layer {

    var window: NSWindow { controller.window! }
    var contentView: NSView { window.contentView! }
    var storyboard: NSStoryboard { NSStoryboard(name: "Onboarding", bundle: nil) }

    var onboardingVC: OnboardingLayerViewController!

    override init(renderer: Renderer) {

        super.init(renderer: renderer)

        onboardingVC = storyboard.instantiateController(withIdentifier: "OnboardingLayerViewController") as? OnboardingLayerViewController
        onboardingVC.view.wantsLayer = true
        onboardingVC.view.layer?.backgroundColor = NSColor.windowBackgroundColor.cgColor
    }

    override func alphaDidChange() {

        onboardingVC.view.alphaValue = CGFloat(alpha.current)

        if alpha.current > 0 && onboardingVC.view.superview == nil {

            contentView.addSubview(onboardingVC.view)

            onboardingVC.view.translatesAutoresizingMaskIntoConstraints = false
            NSLayoutConstraint.activate([
                onboardingVC.view.leadingAnchor.constraint(equalTo: contentView.leadingAnchor),
                onboardingVC.view.trailingAnchor.constraint(equalTo: contentView.trailingAnchor),
                onboardingVC.view.topAnchor.constraint(equalTo: contentView.topAnchor),
                onboardingVC.view.bottomAnchor.constraint(equalTo: contentView.bottomAnchor)
            ])
        }

        if alpha.current == 0 && onboardingVC.view.superview != nil {

            onboardingVC.view.removeFromSuperview()
        }
    }
}

@MainActor
class OnboardingViewController1: NSViewController {

    var model = 0 { didSet { refresh() } }

    var a500: Bool { model == 0 }
    var a1000: Bool { model == 1 }
    var a2000: Bool { model == 2 }

    @IBOutlet weak var a500Button: OnboardingButton!
    @IBOutlet weak var a1000Button: OnboardingButton!
    @IBOutlet weak var a2000Button: OnboardingButton!

    @IBAction func modelAction(_ sender: NSControl) {

        print("modelAction \(sender.tag)")
        model = sender.tag
    }

    override func viewDidLoad() {

        refresh()
    }

    func refresh() {

        print("refresh")

        a500Button.state = a500 ? .on : .off
        a1000Button.state = a1000 ? .on : .off
        a2000Button.state = a2000 ? .on : .off
    }
}

@MainActor
class OnboardingViewController2: NSViewController {

    var rom = 0 { didSet { refresh() } }

    var aros: Bool { rom == 0 }
    var diag: Bool { rom == 1 }

    @IBOutlet weak var arosButton: OnboardingButton!
    @IBOutlet weak var diagButton: OnboardingButton!

    @IBAction func romAction(_ sender: NSControl) {

        print("romAction \(sender.tag)")
        rom = sender.tag
    }

    override func viewDidLoad() {

        refresh()
    }

    func refresh() {

        print("refresh")
        arosButton.state = aros ? .on : .off
        diagButton.state = diag ? .on : .off
    }
}

