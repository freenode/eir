
MODULE = Eir            PACKAGE = Eir::Storage

void
Save(SV *ref, const char *dest)
CODE:
    StorageManager::get_instance()->Save(value_from_sv(ref), dest);
