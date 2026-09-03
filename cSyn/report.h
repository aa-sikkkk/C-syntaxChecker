#ifndef REPORT_H
#define REPORT_H

#include "analysis.h"

// Report generators (shared by CLI and GUI).
// They write self-contained HTML / JSON reports for one analyzed file.
void generate_html_report(const char *filename, const char *source,
                          DiagnosticList *diags, FILE *out);
void generate_json_report(const char *filename, const char *source,
                          DiagnosticList *diags, FILE *out);

#endif