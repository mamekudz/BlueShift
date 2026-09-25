# BlueShift license policy draft

> **POLICY DRAFT — NOT FINAL LICENSE TEXT**  
> This is plain-language intent for review. It is **not** a binding license,
> EULA, or commercial contract. Do not treat this file as legal permission.

**Related:** [`license-audit.md`](license-audit.md) · contact via GitHub (below)

---

## 1. Scope of this draft

This draft describes how the **project author** currently intends to license
**original BlueShift material** (software authored for BlueShift, BlueShift
documentation authored here, and BlueShift branding owned by the author).

It does **not** govern third-party components. Those keep their own licenses
(see `THIRD_PARTY_LICENSES.md` and the license audit).

---

## 2. Free without a commercial BlueShift license (intended)

For original BlueShift material, the author intends to allow **without** a
paid commercial BlueShift license:

- personal use  
- hobby use  
- educational use  
- research  
- non-commercial experimentation  
- building BlueShift hardware for one’s **own** use  
- modifying the author’s BlueShift code for those purposes  
- non-commercial forks / sharing, subject to the **final** published license  

Exact wording, copyleft-or-not, and patent language will appear only in the
final license.

---

## 3. Separate commercial BlueShift license (intended)

A separate commercial agreement with the author would be required for, among
other things:

- manufacturing BlueShift hardware **for sale**  
- selling assembled BlueShift devices  
- selling BlueShift kits **as the product**  
- commercial distribution of BlueShift-based products  
- OEM / ODM use  
- integrating BlueShift into a commercial product  
- offering a BlueShift-derived product commercially  
- other commercial exploitation of original BlueShift material where the final
  license can legally impose that restriction  

**The author welcomes manufacturing and OEM partners.**

Until the final license exists: **commercial use of original BlueShift
material is not granted by this draft.**

---

## 4. Third-party components

BlueShift’s license can only cover rights the author actually holds.

Examples that remain under **their** licenses (non-exhaustive):

- ESP-IDF / Arduino-ESP32 / PlatformIO packages  
- ThingPulse SSD1306 driver  
- Bluepad32 / BTstack / EspBle / ESP32KeyBridge **if ever incorporated**  
- npm / Gulp dependencies  
- µGulp Ready badge artwork (badge rules ≠ firmware license)  

A BlueShift commercial license **does not** replace a BlueKitchen BTstack
commercial license, Espressif notices, or MIT attribution duties.

---

## 5. Branding / name / logo

Software/hardware licensing and **branding** are separate concepts.

Authoritative branding rules: [`branding-policy.md`](branding-policy.md).

Intended summary:

- **BlueShift™** is the project and product name used by Meinolf Amekudzi.  
- The claim applies to the **word mark** “BlueShift” (use **™**, never **®**).  
- Use ™ in prominent public-facing titles and marketing; **not** in technical
  identifiers (`blueshift`, namespaces, filenames, URLs, BLE names by default).  
- Graphical logo / figurative mark: **UNDECIDED** until final artwork is approved.  
- Commercial brand use of BlueShift™ requires agreement; repo visibility alone
  is not a brand license.  
- Do **not** claim a registered trademark.  

---

## 6. Contributions (interim)

Until the final license and contribution terms are published:

- Issues, discussions, and documentation feedback are welcome.  
- **Code contributions should not be merged** into mainline firmware without
  a CLA or other inbound rights agreement suitable for dual licensing.  

See options A/B/C in `license-audit.md`. Interim recommendation: **no outside
code merges** (Option C).

---

## 7. Commercial licensing outline (non-binding)

Possible subjects for a future commercial agreement (no prices, not a contract):

| Topic | Notes |
| --- | --- |
| Manufacturing rights | Produce hardware implementing BlueShift |
| Distribution rights | Sell devices / kits |
| OEM / ODM integration | Embed in another product |
| Branding | Whether and how “BlueShift” / logo may appear |
| Modifications | Allowed changes; upstream contribution expectations |
| Support | Optional; not implied by code access |
| Warranty | Typically limited / AS-IS unless negotiated |
| Territory / term | Negotiated |
| Fee structure | Per-unit, flat, or negotiated — **TBD, no figures here** |

---

## 8. Contact (commercial inquiries)

Use the public GitHub project channels (do not invent private addresses):

- Repository: https://github.com/mamekudz/BlueShift  
- Issues: https://github.com/mamekudz/BlueShift/issues  
- Author profile: https://github.com/mamekudz  

Suggested issue title prefix: `commercial-licensing:`  

Interested in manufacturing, distributing, integrating, or selling BlueShift?
Open a GitHub issue or contact the author via the profile above to discuss
commercial licensing / OEM arrangements.

---

## 9. What this draft deliberately does **not** do

- Create `LICENSE` / `LICENSE.md` / `COMMERCIAL-LICENSE.md` with legal terms  
- Grant commercial rights today  
- Override third-party licenses  
- Claim `BlueShift®` or imply a registered trademark  
- Treat draft logo artwork as a settled figurative mark  
- Set prices  
