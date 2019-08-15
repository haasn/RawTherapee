/*
 *  This file is part of RawTherapee.
 *
 *  Copyright (c) 2004-2010 Gabor Horvath <hgabor@rawtherapee.com>
 *
 *  RawTherapee is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  RawTherapee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RawTherapee.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cmath>
#include "eventmapper.h"
#include "pdsharpening.h"
#include "options.h"

using namespace rtengine;
using namespace rtengine::procparams;

PdSharpening::PdSharpening () : FoldableToolPanel(this, "pdsharpening", M("TP_PDSHARPENING_LABEL"), false, true)
{

    auto m = ProcEventMapper::getInstance();
    EvPdShrContrast = m->newEvent(DEMOSAIC, "HISTORY_MSG_PDSHARPEN_CONTRAST");
    EvPdSharpenGamma = m->newEvent(DEMOSAIC, "HISTORY_MSG_PDSHARPEN_GAMMA");
    EvPdShrDRadius = m->newEvent(DEMOSAIC, "HISTORY_MSG_PDSHARPEN_RADIUS");
    EvPdShrDIterations = m->newEvent(DEMOSAIC, "HISTORY_MSG_PDSHARPEN_ITERATIONS");

    Gtk::HBox* hb = Gtk::manage (new Gtk::HBox ());
    hb->show ();
    contrast = Gtk::manage(new Adjuster (M("TP_SHARPENING_CONTRAST"), 0, 200, 1, 10));
    contrast->setAdjusterListener (this);
    pack_start(*contrast);
    contrast->show();

    pack_start (*hb);

    rld = new Gtk::VBox ();
    gamma = Gtk::manage (new Adjuster (M("TP_SHARPENING_GAMMA"), 0.5, 3.0, 0.05, 1.35));
    dradius = Gtk::manage (new Adjuster (M("TP_SHARPENING_EDRADIUS"), 0.4, 2.5, 0.01, 0.75));
    diter = Gtk::manage (new Adjuster (M("TP_SHARPENING_RLD_ITERATIONS"), 5, 100, 1, 30));
    rld->pack_start (*gamma);
    rld->pack_start (*dradius);
    rld->pack_start (*diter);
    gamma->show();
    dradius->show ();
    diter->show ();
    rld->show ();
    pack_start(*rld);

    dradius->setAdjusterListener (this);
    gamma->setAdjusterListener (this);
    diter->setAdjusterListener (this);

    if (contrast->delay < options.adjusterMaxDelay) {
        contrast->delay = options.adjusterMaxDelay;
    }
    if (dradius->delay < options.adjusterMaxDelay) {
        dradius->delay = options.adjusterMaxDelay;
    }
    if (gamma->delay < options.adjusterMaxDelay) {
        gamma->delay = options.adjusterMaxDelay;
    }
    if (diter->delay < options.adjusterMaxDelay) {
        diter->delay = options.adjusterMaxDelay;
    }

    rld->reference();
}

PdSharpening::~PdSharpening ()
{

    delete rld;
}


void PdSharpening::read (const ProcParams* pp, const ParamsEdited* pedited)
{

    disableListener ();

    if (pedited) {
        contrast->setEditedState    (pedited->pdsharpening.contrast ? Edited : UnEdited);
        gamma->setEditedState       (pedited->pdsharpening.gamma ? Edited : UnEdited);
        dradius->setEditedState     (pedited->pdsharpening.deconvradius ? Edited : UnEdited);
        diter->setEditedState       (pedited->pdsharpening.deconviter ? Edited : UnEdited);

        set_inconsistent                (multiImage && !pedited->pdsharpening.enabled);
    }

    setEnabled(pp->pdsharpening.enabled);

    contrast->setValue      (pp->pdsharpening.contrast);
    gamma->setValue         (pp->pdsharpening.gamma);
    dradius->setValue       (pp->pdsharpening.deconvradius);
    diter->setValue         (pp->pdsharpening.deconviter);

    enableListener ();
}

void PdSharpening::write (ProcParams* pp, ParamsEdited* pedited)
{

    pp->pdsharpening.contrast         = contrast->getValue ();
    pp->pdsharpening.enabled          = getEnabled ();
    pp->pdsharpening.gamma     = gamma->getValue ();
    pp->pdsharpening.deconvradius     = dradius->getValue ();
    pp->pdsharpening.deconviter       = (int)diter->getValue ();

    if (pedited) {
        pedited->pdsharpening.contrast           = contrast->getEditedState ();
        pedited->pdsharpening.gamma       = gamma->getEditedState ();
        pedited->pdsharpening.deconvradius       = dradius->getEditedState ();
        pedited->pdsharpening.deconviter         = diter->getEditedState ();
        pedited->pdsharpening.enabled            = !get_inconsistent();
    }
}

void PdSharpening::setDefaults (const ProcParams* defParams, const ParamsEdited* pedited)
{

    contrast->setDefault (defParams->pdsharpening.contrast);
    gamma->setDefault (defParams->pdsharpening.gamma);
    dradius->setDefault (defParams->pdsharpening.deconvradius);
    diter->setDefault (defParams->pdsharpening.deconviter);

    if (pedited) {
        contrast->setDefaultEditedState     (pedited->pdsharpening.contrast ? Edited : UnEdited);
        gamma->setDefaultEditedState      (pedited->pdsharpening.gamma ? Edited : UnEdited);
        dradius->setDefaultEditedState      (pedited->pdsharpening.deconvradius ? Edited : UnEdited);
        diter->setDefaultEditedState        (pedited->pdsharpening.deconviter ? Edited : UnEdited);
    } else {
        contrast->setDefaultEditedState     (Irrelevant);
        gamma->setDefaultEditedState      (Irrelevant);
        dradius->setDefaultEditedState      (Irrelevant);
        diter->setDefaultEditedState        (Irrelevant);
    }
}

void PdSharpening::adjusterChanged (Adjuster* a, double newval)
{
    if (listener && (multiImage || getEnabled()) ) {

        Glib::ustring costr;

        if (a == gamma || a == dradius) {
            costr = Glib::ustring::format (std::setw(3), std::fixed, std::setprecision(2), a->getValue());
        } else {
            costr = Glib::ustring::format ((int)a->getValue());
        }

        if (a == contrast) {
            listener->panelChanged (EvPdShrContrast, costr);
        } else if (a == gamma) {
            listener->panelChanged (EvPdSharpenGamma, costr);
        } else if (a == dradius) {
            listener->panelChanged (EvPdShrDRadius, costr);
        } else if (a == diter) {
            listener->panelChanged (EvPdShrDIterations, costr);
        }
    }
}

void PdSharpening::enabledChanged ()
{
    if (listener) {
        if (get_inconsistent()) {
            listener->panelChanged (EvPdShrEnabled, M("GENERAL_UNCHANGED"));
        } else if (getEnabled()) {
            listener->panelChanged (EvPdShrEnabled, M("GENERAL_ENABLED"));
        } else {
            listener->panelChanged (EvPdShrEnabled, M("GENERAL_DISABLED"));
        }
    }
}

void PdSharpening::adjusterChanged(ThresholdAdjuster* a, double newBottom, double newTop)
{
}

void PdSharpening::adjusterChanged(ThresholdAdjuster* a, double newBottomLeft, double newTopLeft, double newBottomRight, double newTopRight)
{
}

void PdSharpening::adjusterChanged(ThresholdAdjuster* a, int newBottom, int newTop)
{
}

void PdSharpening::adjusterChanged(ThresholdAdjuster* a, int newBottomLeft, int newTopLeft, int newBottomRight, int newTopRight)
{
}

void PdSharpening::adjusterChanged2(ThresholdAdjuster* a, int newBottomL, int newTopL, int newBottomR, int newTopR)
{
}

void PdSharpening::setBatchMode (bool batchMode)
{

    ToolPanel::setBatchMode (batchMode);

//    pack_start (*rld);

    contrast->showEditedCB ();
    gamma->showEditedCB ();
    dradius->showEditedCB ();
    diter->showEditedCB ();
}

void PdSharpening::setAdjusterBehavior (bool contrastadd, bool gammaadd, bool radiusadd, bool amountadd, bool dampingadd, bool iteradd, bool edgetoladd, bool haloctrladd)
{

    contrast->setAddMode(contrastadd);
    gamma->setAddMode(gammaadd);
    dradius->setAddMode(radiusadd);
    diter->setAddMode(iteradd);
}

void PdSharpening::trimValues (rtengine::procparams::ProcParams* pp)
{

    contrast->trimValue(pp->pdsharpening.contrast);
    gamma->trimValue(pp->pdsharpening.gamma);
    dradius->trimValue(pp->pdsharpening.deconvradius);
    diter->trimValue(pp->pdsharpening.deconviter);
}
