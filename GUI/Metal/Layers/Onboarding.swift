
@MainActor
class OnboardingLayerViewController: NSViewController {

    @IBOutlet weak var pageContainerView: NSView!

    var pageController: NSPageController!
    private var pages: [NSViewController] = []
    private var currentPageIndex: Int = 0

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
