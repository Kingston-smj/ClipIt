#include "picker_panel.h"

// This translation unit exists solely to give Qt's MOC a place to emit
// the meta-object machinery for PickerPanel (vtable, staticMetaObject,
// characterSelected signal body, etc.).  Without it the linker cannot
// resolve references from the subclass moc_*.cpp files.
