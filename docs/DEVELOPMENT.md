# Kori development disclosure

Kori is an ASKQ project.

ASKQ defined the product, workflows, behaviour and user experience, and has
performed hands-on testing in OBS throughout development.

AI tools, including ChatGPT and Codex, have been used extensively to assist
with architecture, code generation, documentation, debugging and repository
preparation. ASKQ reviews the behaviour and approves releases based on direct
testing.

Before a wider release, Kori should also receive independent human code review
and testing across a broader range of OBS installations and source types.

## Tracked post-beta refinement

The current focus picker treats the chosen point as an anchor and keeps it near
its existing canvas position during a zoom. Users can offset the marker to tune
the final composition, but clicking a face does not necessarily place that face
at the exact centre of the finished frame.

After the initial beta, evaluate explicit destination-framing modes:

- **Centre selected point** as the simple default;
- **Keep selected point in place** for the current anchored behaviour; and
- later, optional upper-third or custom destination framing.

The preview should make the destination clear, potentially with a subtle
canvas-centre guide. This is a usability refinement and is not considered a
beta-blocking defect.

Problems can be reported through GitHub Issues or by emailing
**kori.dev@askq.co.nz**.
