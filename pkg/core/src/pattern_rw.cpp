// **************************************************************************
// File       [ pattern_rw.cpp ]
// Author     [ littleshamoo ]
// Synopsis   [ ]
// Date       [ 2011/10/10 created ]
// **************************************************************************

#include "pattern_rw.h"

using namespace std;
using namespace IntfNs;
using namespace CoreNs;

// PatternReader
//{{{ void PatternReader::setPiOrder(const PatNames * const)
// this method map the PI order to the circuit order
// 
void PatternReader::setPiOrder(const PatNames * const pis) {
    if (!success_)
        return;
    if (!cir_ || !cir_->nl_) {
        success_ = false;
        return;
    }
    pcoll_->npi_ = 0;
    PatNames *pi = pis->head;
    while (pi) {
        pcoll_->npi_++;
        pi = pi->next;
    }
    delete [] pcoll_->piOrder_;
    pcoll_->piOrder_ = new int[pcoll_->npi_];
    pi = pis->head;
    int i = 0;
    while (pi) {
        Port *p = cir_->nl_->getTop()->getPort(pi->name);
        if (!p) {
            fprintf(stderr, "**ERROR PatternReader::setPiOrder(): port ");
            fprintf(stderr, "`%s' not found\n", pi->name);
            success_ = false;
            delete [] pcoll_->piOrder_;
            pcoll_->piOrder_ = NULL;
            return;
        }
        pcoll_->piOrder_[i] = cir_->portToGate_[p->id_];
        i++;
        pi = pi->next;
    }
} //}}}
//{{{ void PatternReader::setPpiOrder(const PatNames * const)
void PatternReader::setPpiOrder(const PatNames * const ppis) {
    if (!success_)
        return;
    if (!cir_ || !cir_->nl_) {
        success_ = false;
        return;
    }
    pcoll_->nppi_ = 0;
    PatNames *ppi = ppis->head;
    while (ppi) {
        pcoll_->nppi_++;
        ppi = ppi->next;
    }
    delete [] pcoll_->ppiOrder_;
    pcoll_->ppiOrder_ = new int[pcoll_->nppi_];
    ppi = ppis->head;
    int i = 0;
    while (ppi) {
        Cell *c = cir_->nl_->getTop()->getCell(ppi->name);
        if (!c) {
            fprintf(stderr, "**ERROR PatternReader::setPpiOrder(): gate ");
            fprintf(stderr, "`%s' not found\n", ppi->name);
            success_ = false;
            delete [] pcoll_->ppiOrder_;
            pcoll_->ppiOrder_ = NULL;
            return;
        }
        pcoll_->ppiOrder_[i] = cir_->cellToGate_[c->id_];
        i++;
        ppi = ppi->next;
    }
} //}}}
//{{{ void PatternReader::setPoOrder(const PatNames * const)
void PatternReader::setPoOrder(const PatNames * const pos) {
    if (!success_)
        return;
    if (!cir_ || !cir_->nl_) {
        success_ = false;
        return;
    }
    pcoll_->npo_ = 0;
    PatNames *po = pos->head;
    while (po) {
        pcoll_->npo_++;
        po = po->next;
    }
    delete [] pcoll_->poOrder_;
    pcoll_->poOrder_ = new int[pcoll_->npo_];
    po = pos->head;
    int i = 0;
    while (po) {
        Port *p = cir_->nl_->getTop()->getPort(po->name);
        if (!p) {
            fprintf(stderr, "**ERROR PatternReader::setPoOrder(): port ");
            fprintf(stderr, "`%s' not found\n", po->name);
            success_ = false;
            delete [] pcoll_->poOrder_;
            pcoll_->poOrder_ = NULL;
            return;
        }
        pcoll_->poOrder_[i] = cir_->portToGate_[p->id_];
        i++;
        po = po->next;
    }
} //}}}
//{{{ void PatternReader::setPatternType(const PatType &)
void PatternReader::setPatternType(const PatType &type) {
    if (!success_)
        return;
    switch (type) {
        case IntfNs::BASIC_SCAN:
            pcoll_->type_ = PatternProcessor::BASIC_SCAN;
            break;
        case IntfNs::LAUNCH_CAPTURE:
            pcoll_->type_ = PatternProcessor::LAUNCH_CAPTURE;
            break;
        case IntfNs::LAUNCH_SHIFT:
            pcoll_->type_ = PatternProcessor::LAUNCH_SHIFT;
            pcoll_->nsi_ = 1;
            break;
    }
} //}}}
//{{{ void PatternReader::setPatternNum(const int &)
void PatternReader::setPatternNum(const int &num) {
    if (!success_)
        return;
    pcoll_->pats_.resize((size_t)num);
    Pattern *pats = new Pattern[pcoll_->pats_.size()];
    for (size_t i = 0; i < pcoll_->pats_.size(); ++i)
        pcoll_->pats_[i] = &pats[i];
    curPat_ = 0;
} //}}}
//{{{ void PatternReader::addPattern()
// read in a pattern
void PatternReader::addPattern(const char * const pi1,
                                const char * const pi2,
                                const char * const ppi,
                                const char * const si,
                                const char * const po1,
                                const char * const po2,
                                const char * const ppo) {
    if (!success_)
        return;
    if (pi1 && curPat_ < (int)pcoll_->pats_.size()) {
        pcoll_->pats_[curPat_]->pi1_ = new Value[pcoll_->npi_];
        assignValue(pcoll_->pats_[curPat_]->pi1_, pi1, pcoll_->npi_);
    }
    if (pi2 && curPat_ < (int)pcoll_->pats_.size()) {
        pcoll_->pats_[curPat_]->pi2_ = new Value[pcoll_->npi_];
        assignValue(pcoll_->pats_[curPat_]->pi2_, pi2, pcoll_->npi_);
    }
    if (ppi && curPat_ < (int)pcoll_->pats_.size()) {
        pcoll_->pats_[curPat_]->ppi_ = new Value[pcoll_->nppi_];
        assignValue(pcoll_->pats_[curPat_]->ppi_, ppi, pcoll_->nppi_);
    }
    if (si && curPat_ < (int)pcoll_->pats_.size()) {
        pcoll_->pats_[curPat_]->si_ = new Value[pcoll_->nsi_];
        assignValue(pcoll_->pats_[curPat_]->si_, si, pcoll_->nsi_);
    }
    if (po1 && curPat_ < (int)pcoll_->pats_.size()) {
        pcoll_->pats_[curPat_]->po1_ = new Value[pcoll_->npo_];
        assignValue(pcoll_->pats_[curPat_]->po1_, po1, pcoll_->npo_);
    }
    if (po2 && curPat_ < (int)pcoll_->pats_.size()) {
        pcoll_->pats_[curPat_]->po2_ = new Value[pcoll_->npo_];
        assignValue(pcoll_->pats_[curPat_]->po2_, po2, pcoll_->npo_);
    }
    if (ppo && curPat_ < (int)pcoll_->pats_.size()) {
        pcoll_->pats_[curPat_]->ppo_ = new Value[pcoll_->nppi_];
        assignValue(pcoll_->pats_[curPat_]->ppo_, ppo, pcoll_->nppi_);
    }
    curPat_++;
} //}}}
//{{{ void PatternReader::assignValue()
void PatternReader::assignValue(Value *v, const char * const pat,
                                 const int &size) {
    for (int i = 0; i < size; ++i) {
        if (pat[i] == '0')
            v[i] = L;
        else if (pat[i] == '1')
            v[i] = H;
        else
            v[i] = X;
    }
} //}}}

// PatternWriter
//{{{ bool PatternWriter::writePat(const char * const)

// write to LaDS's own *.pat  pattern format 
// support 2 time frames, but no more than 2 time frames
//
bool PatternWriter::writePat(const char * const fname) {
    FILE *fout = fopen(fname, "w");
    if (!fout) {
        fprintf(stderr, "**ERROR PatternWriter::writePat(): file");
        fprintf(stderr, "`%s' cannot be opened\n", fname);
        return false;
    }

    for (int i = 0; i < cir_->npi_; ++i) {
        fprintf(fout, "%s ",
                cir_->nl_->getTop()->getPort(cir_->gates_[i].cid_)->name_);
    }
    fprintf(fout, " |\n");

    for (int i = cir_->npi_; i < cir_->npi_ + cir_->nppi_; ++i) {
        fprintf(fout, "%s ",
                cir_->nl_->getTop()->getCell(cir_->gates_[i].cid_)->name_);
    }
    fprintf(fout, " |\n");

    int start = cir_->npi_ + cir_->nppi_ + cir_->ncomb_;
    for (int i = start; i < start + cir_->npo_; ++i) {
        fprintf(fout, "%s ",
                cir_->nl_->getTop()->getPort(cir_->gates_[i].cid_)->name_);
    }
    fprintf(fout, "\n");

    switch (pcoll_->type_) {
        case PatternProcessor::BASIC_SCAN:
            fprintf(fout, "BASIC_SCAN\n");
            break;
        case PatternProcessor::LAUNCH_CAPTURE:
            fprintf(fout, "LAUNCH_ON_CAPTURE\n");
            break;
        case PatternProcessor::LAUNCH_SHIFT:
            fprintf(fout, "LAUNCH_ON_SHIFT\n");
            break;
    }

    fprintf(fout, "_num_of_pattern_%d\n", (int)pcoll_->pats_.size());

    for (int i = 0; i < (int)pcoll_->pats_.size(); ++i) {
        fprintf(fout, "_pattern_%d ", i + 1);
        if (pcoll_->pats_[i]->pi1_) {
            for (int j = 0; j < pcoll_->npi_; ++j) {
                if (pcoll_->pats_[i]->pi1_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->pi1_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        fprintf(fout, " | ");
        if (pcoll_->pats_[i]->pi2_) {
            for (int j = 0; j < pcoll_->npi_; ++j) {
                if (pcoll_->pats_[i]->pi2_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->pi2_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        fprintf(fout, " | ");
        if (pcoll_->pats_[i]->ppi_) {
            for (int j = 0; j < pcoll_->nppi_; ++j) {
                if (pcoll_->pats_[i]->ppi_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->ppi_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");    
            }
        }
        fprintf(fout, " | ");
        if (pcoll_->pats_[i]->si_) {
            for (int j = 0; j < pcoll_->nsi_; ++j) {
                if (pcoll_->pats_[i]->si_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->si_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        fprintf(fout, " | ");
        if (pcoll_->pats_[i]->po1_) {
            for (int j = 0; j < pcoll_->npo_; ++j) {
                if (pcoll_->pats_[i]->po1_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->po1_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        fprintf(fout, " | ");
        if (pcoll_->pats_[i]->po2_) {
            for (int j = 0; j < pcoll_->npo_; ++j) {
                if (pcoll_->pats_[i]->po2_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->po2_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        fprintf(fout, " | ");
        if (pcoll_->pats_[i]->ppi_) {
            for (int j = 0; j < pcoll_->nppi_; ++j) {
                if (pcoll_->pats_[i]->ppo_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->ppo_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        fprintf(fout, "\n");
    }
    fprintf(fout, "\n");
    fclose(fout);

    return true;
} //}}}

//{{{ bool PatternWriter::writeLht(const char * const)

// write to Ling Hsio-Ting's pattern format
// This format is OBSOLETE, no longer supported
//  
bool PatternWriter::writeLht(const char * const fname) {
    FILE *fout = fopen(fname, "w");
    if (!fout) {
        fprintf(stderr, "**ERROR PatternWriter::writePat(): file");
        fprintf(stderr, "`%s' cannot be opened\n", fname);
        return false;
    }

    for (size_t i = 0; i < pcoll_->pats_.size(); ++i) {
        fprintf(fout, "%d: ", (int)i + 1);
        if (pcoll_->pats_[i]->pi1_) {
            for (int j = 0; j < pcoll_->npi_; ++j) {
                if (pcoll_->pats_[i]->pi1_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->pi1_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        if (pcoll_->pats_[i]->pi2_) {
            fprintf(fout, "->");
            for (int j = 0; j < pcoll_->npi_; ++j) {
                if (pcoll_->pats_[i]->pi2_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->pi2_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        fprintf(fout, "_");
        if (pcoll_->pats_[i]->ppi_) {
            fprintf(fout, "->");
            for (int j = 0; j < pcoll_->nppi_; ++j) {
                if (pcoll_->pats_[i]->ppi_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->ppi_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        if (pcoll_->pats_[i]->si_) {
            fprintf(fout, "@");
            for (int j = 0; j < pcoll_->nsi_; ++j) {
                if (pcoll_->pats_[i]->si_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->si_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        fprintf(fout, " | ");
        if (pcoll_->pats_[i]->po1_
            && pcoll_->type_ == PatternProcessor::BASIC_SCAN) {
            for (int j = 0; j < pcoll_->npo_; ++j) {
                if (pcoll_->pats_[i]->po1_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->po1_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        if (pcoll_->pats_[i]->po2_
            && (pcoll_->type_ == PatternProcessor::LAUNCH_CAPTURE
                ||pcoll_->type_ == PatternProcessor::LAUNCH_SHIFT)) {
            for (int j = 0; j < pcoll_->npo_; ++j) {
                if (pcoll_->pats_[i]->po2_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->po2_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        fprintf(fout, "_");
        if (pcoll_->pats_[i]->ppo_) {
            fprintf(fout, "->");
            for (int j = 0; j < pcoll_->nppi_; ++j) {
                if (pcoll_->pats_[i]->ppo_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->ppo_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
        }
        fprintf(fout, "\n");
    }
    fprintf(fout, "\n");
    fclose(fout);

    return true;
} //}}}

// write to Mentor ASCii
// E.5 problem
// must test with mentor fastscan
bool PatternWriter::writeAscii(const char * const fname) {
    FILE *fout = fopen(fname, "w");
    int first_flag = 1;
    int seqCircuitCheck = 0;
    if (!fout) {
        fprintf(stderr, "**ERROR PatternWriter::writePat(): file");
        fprintf(stderr, "`%s' cannot be opened\n", fname);
        return false;
    }
    fprintf(fout, "ASCII_PATTERN_FILE_VERSION = 2;\n");
    fprintf(fout, "SETUP =\n");

    // input
    fprintf(fout, "declare input bus \"PI\" = ");
    //fprintf(fout, "\"/CK\", \"/test_si\", \"/test_se\"");
    for (size_t i = 0; i < cir_->nl_->getTop()->getNPort(); ++i) {
        Port *p = cir_->nl_->getTop()->getPort(i);
        if (p->type_ != Port::INPUT) continue;
        if (first_flag) {
            fprintf(fout, "\"/%s\"",
                    cir_->nl_->getTop()->getPort(i)->name_);
            first_flag = 0;
        } else { 
            fprintf(fout, ", \"/%s\"",
                    cir_->nl_->getTop()->getPort(i)->name_);
        }
        if (!strcmp(p->name_,"CK"))
            seqCircuitCheck = 1;
    }
    fprintf(fout, ";\n");
    first_flag = 1;
    // output
    fprintf(fout, "declare output bus \"PO\" = "); 
    for (size_t i = 0; i < cir_->nl_->getTop()->getNPort(); ++i) {
        Port *p = cir_->nl_->getTop()->getPort(i);
        if (p->type_ != Port::OUTPUT) continue;
        if (first_flag) {
            fprintf(fout, "\"/%s\"",
                    cir_->nl_->getTop()->getPort(i)->name_);
            first_flag = 0;
        } else { 
            fprintf(fout, ", \"/%s\"",
                    cir_->nl_->getTop()->getPort(i)->name_);
        }
    }
    fprintf(fout, ";\n");

    // clock
    if (seqCircuitCheck) {
        fprintf(fout, "clock \"/CK\" =\n");
        fprintf(fout, "    off_state = 0;\n");
        fprintf(fout, "    pulse_width = 1;\n");
        fprintf(fout, "end;\n");
    
        // test setup
        fprintf(fout, "procedure test_setup \"test_setup\" =\n");
        fprintf(fout, "    force \"/CK\" 0 0;\n");
        fprintf(fout, "    force \"/test_si\" 0 0;\n");
        fprintf(fout, "    force \"/test_se\" 0 0;\n");
        fprintf(fout, "end;\n");

        // scan group
        fprintf(fout, "scan_group \"group1\" =\n");
        fprintf(fout, "    scan_chain \"chain1\" =\n");
        fprintf(fout, "    scan_in = \"/test_si\";\n");
        fprintf(fout, "    scan_out = \"/test_so\";\n");
        fprintf(fout, "    length = %d;\n", cir_->nppi_);
        fprintf(fout, "    end;\n");

        fprintf(fout, "    procedure shift \"group1_load_shift\" =\n");
        fprintf(fout, "    force_sci \"chain1\" 0;\n");
        fprintf(fout, "    force \"/CK\" 1 16;\n");
        fprintf(fout, "    force \"/CK\" 0 19;\n");
        fprintf(fout, "    period 32;\n");
        fprintf(fout, "    end;\n");

        fprintf(fout, "    procedure shift \"group1_unload_shift\" =\n");
        fprintf(fout, "    measure_sco \"chain1\" 0;\n");
        fprintf(fout, "    force \"/CK\" 1 16;\n");
        fprintf(fout, "    force \"/CK\" 0 19;\n");
        fprintf(fout, "    period 32;\n");
        fprintf(fout, "    end;\n");

        fprintf(fout, "    procedure load \"group1_load\" =\n");
        fprintf(fout, "    force \"/CK\" 0 0;\n");
        fprintf(fout, "    force \"/test_se\" 1 0;\n");
        fprintf(fout, "    force \"/test_si\" 0 0;\n");
        fprintf(fout, "    apply \"group1_load_shift\" %d 32;\n", cir_->nppi_);
        fprintf(fout, "    end;\n");

        fprintf(fout, "    procedure unload \"group1_unload\" =\n");
        fprintf(fout, "    force \"/CK\" 0 0;\n");
        fprintf(fout, "    force \"/test_se\" 1 0;\n");
        fprintf(fout, "    force \"/test_si\" 0 0;\n");
        fprintf(fout, "    apply \"group1_unload_shift\" %d 32;\n", cir_->nppi_);
        fprintf(fout, "    end;\n");

        fprintf(fout, "end;\n");
    }
    fprintf(fout, "end;\n\n");

    fprintf(fout, "SCAN_TEST =\n");
    //fprintf(fout, "apply \"test_setup\" 1 0;\n");

    for (int i = 0; i < (int)pcoll_->pats_.size(); ++i) {
        fprintf(fout, "pattern = %d", i);
        if (pcoll_->pats_[i]->pi2_)
            fprintf(fout, " clock_sequential;\n");
        else
            fprintf(fout, ";\n");

        if (pcoll_->nppi_) {
            fprintf(fout, "apply  \"group1_load\" 0 =\n");
            fprintf(fout, "chain \"chain1\" = \"");
            for (int j = pcoll_->nppi_ -1; j >= 0; --j) {
                if (pcoll_->pats_[i]->ppi_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->ppi_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
            fprintf(fout, "\";\n");
            fprintf(fout, "end;\n");
        }
        if (pcoll_->pats_[i]->pi1_) {
            fprintf(fout, "force \"PI\" \"");
            if (seqCircuitCheck) 
                fprintf(fout, "000");
            for (int j = 0; j < pcoll_->npi_; ++j) {
                if (pcoll_->pats_[i]->pi1_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->pi1_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
            fprintf(fout, "\" 1;\n");
        }
        //fprintf(fout, "pulse \"/CK\" 2;\n");
        if (pcoll_->pats_[i]->pi2_) {
            if (seqCircuitCheck)
                fprintf(fout, "pulse \"/CK\" 2;\n");
            fprintf(fout, "force \"PI\" \"");
            if (seqCircuitCheck) 
                fprintf(fout, "000");
            for (int j = 0; j < pcoll_->npi_; ++j) {
                if (pcoll_->pats_[i]->pi2_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->pi2_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
            fprintf(fout, "\" 3;\n");
        }
        if (pcoll_->pats_[i]->po2_) {
            fprintf(fout, "measure \"PO\" \"");
            if (seqCircuitCheck) 
                fprintf(fout, "X");
            for (int j = 0; j < pcoll_->npo_; ++j) {
                if (pcoll_->pats_[i]->po2_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->po2_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
            fprintf(fout, "\" 4;\n");
        }
        else if (pcoll_->pats_[i]->po1_) {
            fprintf(fout, "measure \"PO\" \"");
            if (seqCircuitCheck) 
                fprintf(fout, "X");
            for (int j = 0; j < pcoll_->npo_; ++j) {
                if (pcoll_->pats_[i]->po1_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->po1_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
            fprintf(fout, "\" 4;\n");
        }
        /*if (pcoll_->pats_[i]->po2_) {
            fprintf(fout, "pulse \"/CK\" 5;\n");
            fprintf(fout, "measure \"PO\" \"");
            if (seqCircuitCheck) 
                fprintf(fout, "X");
            for (int j = 0; j < pcoll_->npo_; ++j) {
                if (pcoll_->pats_[i]->po2_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->po2_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
            fprintf(fout, "\" 6;\n");
        }*/

        if (pcoll_->nppi_) {
            fprintf(fout, "pulse \"/CK\" 5;\n");
            fprintf(fout, "apply  \"group1_unload\" 6 =\n");
            fprintf(fout, "chain \"chain1\" = \"");
            for (int j = pcoll_->nppi_ - 1; j >= 0; --j) {
                if (pcoll_->pats_[i]->ppo_[j] == L)
                    fprintf(fout, "0");
                else if (pcoll_->pats_[i]->ppo_[j] == H)
                    fprintf(fout, "1");
                else
                    fprintf(fout, "X");
            }
            fprintf(fout, "\";\n");
            fprintf(fout, "end;\n");
        }
    }
    fprintf(fout, "end;\n");

    // scan cells
    if(seqCircuitCheck) {    
        fprintf(fout, "SCAN_CELLS =\n");
        fprintf(fout, "scan_group \"group1\" =\n");
        fprintf(fout, "scan_chain \"chain1\" =\n");
        for (int i = cir_->npi_ + cir_->nppi_ - 1; i >= cir_->npi_; --i) {
            fprintf(fout,
                    "scan_cell = %d MASTER FFFF \"/%s\" \"I1\" \"SI\" \"Q\";\n",
                    cir_->npi_ + cir_->nppi_ - 1 - i, cir_->nl_->getTop()->getCell(cir_->gates_[i].cid_)->name_);
        }
    

    fprintf(fout, "end;\n");
    fprintf(fout, "end;\n");
    fprintf(fout, "end;\n");
    }
    fprintf(fout, "\n");
    fclose(fout);

    return true;
}

bool ProcedureWriter::writeProc(const char * const fname) {
    FILE *fout = fopen(fname, "w");
    if (!fout) {
        fprintf(stderr, "**ERROR PatternWriter::writePat(): file");
        fprintf(stderr, "`%s' cannot be opened\n", fname);
        return false;
    }

    fprintf(fout, "set time scale 1.000000 ns ;\n");
    fprintf(fout, "set strobe_window time 3 ;\n\n");
    fprintf(fout, "timeplate _default_WFT_ =\n");
    fprintf(fout, "force_pi 0 ;\n");
    fprintf(fout, "measure_po 29 ;\n");
    fprintf(fout, "pulse CK 16 3;\n");
    fprintf(fout, "period 32 ;\n");
    fprintf(fout, "end;\n\n");
    fprintf(fout, "procedure shift =\n");
    fprintf(fout, "scan_group group1 ;\n");
    fprintf(fout, "timeplate _default_WFT_ ;\n");
    fprintf(fout, "cycle =\n");
    fprintf(fout, "force_sci ;\n");
    fprintf(fout, "pulse CK ;\n");
    fprintf(fout, "measure_sco ;\n");
    fprintf(fout, "end;\n");
    fprintf(fout, "end;\n\n");
    fprintf(fout, "procedure load_unload =\n");
    fprintf(fout, "scan_group group1 ;\n");
    fprintf(fout, "timeplate _default_WFT_ ;\n");
    fprintf(fout, "cycle =\n");
    fprintf(fout, "force CK 0 ;\n");
    fprintf(fout, "force test_se 1 ;\n");
    fprintf(fout, "force test_si 0 ;\n");
    fprintf(fout, "measure_sco ;\n");
    fprintf(fout, "end ;\n");
    fprintf(fout, "apply shift %d;\n", cir_->nppi_);
    fprintf(fout, "end;\n");
    fprintf(fout, "procedure test_setup =\n");
    fprintf(fout, "timeplate _default_WFT_ ;\n");
    fprintf(fout, "cycle =\n");
    fprintf(fout, "force CK 0 ;\n");
    fprintf(fout, "force test_se 0 ;\n");
    fprintf(fout, "end;\n");
    fprintf(fout, "end;\n");
    fclose(fout);
    return true;
}


