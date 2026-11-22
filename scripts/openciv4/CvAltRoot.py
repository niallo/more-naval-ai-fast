"""Give BUG the same Beyond the Sword profile path as the native engine."""

import os

documents = os.environ.get("OC4_DOCUMENTS") or os.path.expanduser("~/Documents")
rootDir = os.path.join(documents, "My Games", "Beyond the Sword")
