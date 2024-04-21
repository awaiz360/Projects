import '/flutter_flow/flutter_flow_util.dart';
import 'psychiatrist_panel_widget.dart' show PsychiatristPanelWidget;
import 'package:flutter/material.dart';

class PsychiatristPanelModel extends FlutterFlowModel<PsychiatristPanelWidget> {
  ///  State fields for stateful widgets in this page.

  final unfocusNode = FocusNode();

  @override
  void initState(BuildContext context) {}

  @override
  void dispose() {
    unfocusNode.dispose();
  }
}
