// BlueShift™ T-Lion mechanical reference envelopes (NOT a finished enclosure).
// Units: millimetres. Coordinate system: see 3dprint/ENCLOSURE-SPEC.md / board.json.
//
// Status:
//   PCB outline 108.51 x 29.01 — DOCUMENTED (LilyGO size)
//   Feature positions — PROVISIONAL spatial zones for Plasticity planning
//
// Export: OpenSCAD → STL, or FreeCAD/OpenSCAD → STEP into ../step/

$fn = 32;

pcb_w = 29.01;   // X
pcb_l = 108.51;  // Y
pcb_t = 1.6;     // Z into board — PROVISIONAL laminate

// --- PCB slab (top at Z=0) ---
module pcb() {
    color("forestgreen")
        translate([0, 0, -pcb_t])
            cube([pcb_w, pcb_l, pcb_t]);
}

// OLED module envelope near +Y — PROVISIONAL placement
module oled_module() {
    ow = 27.5; oh = 27.5; ot = 4.0; // DERIVED/PROVISIONAL
    color("black")
        translate([(pcb_w - ow) / 2, pcb_l - oh - 2, 0])
            cube([ow, oh, ot]);
}

// Visible AA opening candidate (for visual check only)
module oled_active_hint() {
    aw = 21.7; ah = 10.9;
    color("dodgerblue", 0.5)
        translate([(pcb_w - aw) / 2, pcb_l - 2 - 27.5 / 2 - ah / 2, 4.01])
            cube([aw, ah, 0.2]);
}

// 5-way actuator zone — PROVISIONAL
module fiveway_zone() {
    color("silver")
        translate([pcb_w / 2, pcb_l - 18, 0])
            cylinder(h = 5, d = 8);
}

// Micro-USB shell zone at -Y — PROVISIONAL
module usb_zone() {
    color("slategray")
        translate([(pcb_w - 8) / 2, -1.5, -0.5])
            cube([8, 5.5, 3]);
}

// 18650 cell envelope (centered along length) — form DOCUMENTED
module cell_18650() {
    color("steelblue", 0.6)
        translate([pcb_w / 2, pcb_l / 2, -9])
            rotate([0, 90, 0])
                cylinder(h = 65, d = 18, center = true);
}

// Antenna keep-out box near +Y tip — PROVISIONAL
module antenna_keepout() {
    color("red", 0.25)
        translate([0, pcb_l - 20, -2])
            cube([pcb_w, 20, 12]);
}

// BAT switch access hint near mid/-Y — PROVISIONAL
module bat_switch_hint() {
    color("orange")
        translate([2, 12, 0])
            cube([6, 4, 3]);
}

pcb();
oled_module();
oled_active_hint();
fiveway_zone();
usb_zone();
cell_18650();
antenna_keepout();
bat_switch_hint();
