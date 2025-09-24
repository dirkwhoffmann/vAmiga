
@MainActor
class TutorialLayerViewController: NSViewController {

    @IBOutlet weak var pageContainerView: NSView!

    var pageController: NSPageController!

    private var pages: [NSViewController] = []
    private var currentPageIndex: Int = 0

    // var objects: [String] = ["Step1", "Step2"]

    override func viewDidLoad() {

        super.viewDidLoad()
    }

    override func viewDidAppear() {

        func instantiate(_ id: String) -> NSViewController {

            let storyboard = NSStoryboard(name: "Tutorial", bundle: nil)
            return storyboard.instantiateController(withIdentifier: id) as! NSViewController
        }

        super.viewDidAppear()
        pages = [instantiate("Step1"), instantiate("Step2")]
        showPage(at: 0)
    }

    private func showPage(at index: Int, animated: Bool = true) {

        guard pages.indices.contains(index) else { return }

        let newPage = pages[index]

        if children.isEmpty {
            // First page, just add it
            addChild(newPage)
            newPage.view.frame = pageContainerView.bounds
            newPage.view.autoresizingMask = [.width, .height]
            pageContainerView.addSubview(newPage.view)
            currentPageIndex = index
            return
        }

        let oldPage = children[0]

        addChild(newPage)
        newPage.view.frame = pageContainerView.bounds
        newPage.view.autoresizingMask = [.width, .height]

        let options: NSViewController.TransitionOptions =
        [index > currentPageIndex ? .slideLeft : .slideRight, .allowUserInteraction]

        if animated {
            NSAnimationContext.runAnimationGroup({ context in
                context.duration = 1.0
                context.timingFunction = CAMediaTimingFunction(name: .easeInEaseOut)
                self.transition(from: oldPage,
                                to: newPage,
                                options: options,
                                completionHandler: nil)
            }, completionHandler: {
                Task { @MainActor in
                    oldPage.removeFromParent()
                    self.currentPageIndex = index
                }
            })
        } else {
            transition(from: oldPage, to: newPage, options: [], completionHandler: nil)
            oldPage.removeFromParent()
            currentPageIndex = index
        }
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
class Tutorial: Layer {

    var window: NSWindow { controller.window! }
    var contentView: NSView { window.contentView! }
    var storyboard: NSStoryboard { NSStoryboard(name: "Tutorial", bundle: nil) }

    var tutorialVC: TutorialLayerViewController!

    override init(renderer: Renderer) {

        super.init(renderer: renderer)

        tutorialVC = storyboard.instantiateController(withIdentifier: "TutorialLayerViewController") as? TutorialLayerViewController
        tutorialVC.view.wantsLayer = true
        tutorialVC.view.layer?.backgroundColor = NSColor.windowBackgroundColor.cgColor
    }

    override func alphaDidChange() {

        tutorialVC.view.alphaValue = CGFloat(alpha.current)

        if alpha.current > 0 && tutorialVC.view.superview == nil {

            contentView.addSubview(tutorialVC.view)

            /*
            tutorialVC.view.frame = contentView.bounds
            tutorialVC.view.autoresizingMask = [.width, .height]
            tutorialVC.view.needsLayout = true
            tutorialVC.view.layoutSubtreeIfNeeded()
             */
            tutorialVC.view.translatesAutoresizingMaskIntoConstraints = false
            contentView.addSubview(tutorialVC.view)

            NSLayoutConstraint.activate([
                tutorialVC.view.leadingAnchor.constraint(equalTo: contentView.leadingAnchor),
                tutorialVC.view.trailingAnchor.constraint(equalTo: contentView.trailingAnchor),
                tutorialVC.view.topAnchor.constraint(equalTo: contentView.topAnchor),
                tutorialVC.view.bottomAnchor.constraint(equalTo: contentView.bottomAnchor)
            ])

            // Force immediate layout
            contentView.layoutSubtreeIfNeeded()

            // tutorialVC.setupPageController()
        }

        if alpha.current == 0 && tutorialVC.view.superview != nil {
            tutorialVC.view.removeFromSuperview()
        }
    }
}
